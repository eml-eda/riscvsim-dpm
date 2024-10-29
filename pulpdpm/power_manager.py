import gvsoc.systree as gsys
import json


def add_ports(component_list, srcpath):
    interfaces_generated = ""
    ports_generated = ""
    voltage_offsets_generated = ""
    event_handlers=""
    event_handlers_def=""
    events=""
    delay_registers=""
    vcd_signals=""
    event_constructors=""
    event_handlers=""
    delay_offsets=""
    config_delay_offsets=""
    addr = 0
    addr_offsets = """// defined states in power manager
#define off 0x0
#define on_clock_gated 0x1
#define on 0x3

// offset to control the power measurement
#define start_capture 0x1
#define stop_capture 0

//offsets of the config registers
#define on_off_offset 0
#define off_on_offset 1
#define on_cg_offset 2
#define cg_on_offset 3

//define pm addresses mapped to components
"""
    # scans the component list and adds power and voltage port on the class
    for component in component_list:
        power_port_name = "o_POWER_CTRL_" + component
        voltage_port_name = "o_VOLTAGE_CTRL_" + component

        def power_ports(self, itf: gsys.SlaveItf, name=f"power_ctrl_{component}"):
            self.itf_bind(name, itf, signature="wire<int>")

        def voltage_ports(self, itf: gsys.SlaveItf, name=f"voltage_ctrl_{component}"):
            self.itf_bind(name, itf, signature="wire<int>")

        setattr(PowerManager, power_port_name, power_ports)
        setattr(PowerManager, voltage_port_name, voltage_ports)

        # port needs to be added also in the cpp file.
        events= events + f"\tTimeEvent delay_{component};\n"

        event_handlers_def = event_handlers_def + f"\tstatic void {component}_delay_handler(vp::Block *__this, vp::TimeEvent *event);\n"

        interfaces_generated = (
            interfaces_generated
            + f"\tWireMaster<int> power_ctrl_itf_{component};\n\tWireMaster<double> voltage_ctrl_itf_{component};\n"
        )

        delay_registers= delay_registers + f"\tunsigned int {component}_delays[4] = {{1,1,1,1}};\n\tint {component}_next_state;\n"

        vcd_signals= vcd_signals + f"\tvp::Signal<int> {component}_state;\n"

        event_constructors= event_constructors + f"\t, delay_{component}(this, {component}_delay_handler), {component}_state(*this, \"{component}_state\", 3) "

        ports_generated = (
            ports_generated
            + f'\tthis->new_master_port("power_ctrl_{component}", &this->power_ctrl_itf_{component});\n\tthis->new_master_port("voltage_ctrl_{component}", &this->voltage_ctrl_itf_{component});\n'
        )

        event_handlers=event_handlers+f"void PowerManager::{component}_delay_handler(vp::Block *__this, vp::TimeEvent *event)\n{{\n\tPowerManager *_this = (PowerManager *)__this;\n\t_this->power_ctrl_itf_{component}.sync(_this->{component}_next_state);\n\t_this->trace.msg(vp::TraceLevel::DEBUG, \"switching power state of {component} to %s\\n\", statename[_this->{component}_next_state]);\n_this->{component}_state.set(_this->{component}_next_state);\n}}\n"

        delay_offsets = delay_offsets + f"""
        case {addr*4}:
			if (!_this->delay_{component}.is_enqueued())
			{{
				_this->{component}_next_state = power_state;
				// if next state is on check previous state
				if (power_state == ON)
				{{
					if (_this->{component}_state.get() == OFF)
						picoseconds = _this->{component}_delays[1];//off-on
					else
						picoseconds = _this->{component}_delays[3];//cg-on
				}}
				else if (power_state == OFF)
				{{
					picoseconds = _this->{component}_delays[0];//on-off
				}}
				else
					picoseconds = _this->{component}_delays[2];//on-cg

				_this->delay_{component}.enqueue(picoseconds);
			}}
			else
				_this->trace.msg(vp::TraceLevel::DEBUG, "Last change of {component} is still  in progress...\\n");
			break;
"""

        voltage_offsets_generated = (
            voltage_offsets_generated
            + f'\t\tcase {addr*4}:\n\t\t\t_this->voltage_ctrl_itf_{component}.sync(voltage);\n\t\t\t_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of {component} to %f\\n");\n\t\t\tbreak;\n'
        )

        config_delay_offsets = config_delay_offsets + f"""
        case {addr*16}:
		case {addr*16+4}:
		case {addr*16+8}:
		case {addr*16+12}:
			_this->{component}_delays[(addr-{addr*16})/4] = value;
			_this->trace.msg(vp::TraceLevel::DEBUG, "New configuration is: on-off: %d, off-on: %d, on-cg: %d, cg-on: %d\\n", _this->{component}_delays[0],_this->{component}_delays[1],_this->{component}_delays[2],_this->{component}_delays[3]);
		break;
"""

        addr_offsets = addr_offsets + f"#define {component}_offset {addr}\n#define {component}_config_offset {addr*4}\n"
        addr = addr + 1

    # write offsets to a header file
    with open(srcpath.replace("power_manager.cpp", "pm_addr.h"), "w") as f:
        f.writelines(addr_offsets)

    # updates the source file template
    with open(srcpath, "r") as f:
        lines = f.readlines()
    result = []

    in_block = False

    for line in lines:

        if "// GENERATED EVENTS" in line:
            result.append(line)
            result.extend(events)
            in_block = True
            continue

        if "// END GENERATED EVENTS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED EVENT HANDLER DEFINITIONS" in line:
            result.append(line)
            result.extend(event_handlers_def)
            in_block = True
            continue

        if "// END EVENT HANDLER DEFNITIONS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED INTERFACES" in line:
            result.append(line)
            result.extend(interfaces_generated)
            in_block = True
            continue

        if "// END INTERFACES" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED DELAY REGISTERS" in line:
            result.append(line)
            result.extend(delay_registers)
            in_block = True
            continue

        if "// END DELAY REGISTERS" in line:
            in_block = False
            result.append("\n" + line)
            continue 
        
        if "// GENERATED VCD PS SIGNAL" in line:
            result.append(line)
            result.extend(vcd_signals)
            in_block = True
            continue

        if "// END GENERATED VCD PS SIGNAL" in line:
            in_block = False
            result.append("\n" + line)
            continue 

        if "// GENERATED EVENT CONSTRUCTORS" in line:
            result.append(line)
            result.extend(event_constructors)
            in_block = True
            continue

        if "// END EVENT CONSTRUCTORS" in line:
            in_block = False
            result.append("\n" + line)
            continue              

        if "// GENERATED POWER AND VOLTAGE PORTS" in line:
            result.append(line)
            result.extend(ports_generated)
            in_block = True
            continue

        if "// END GENERATED PORTS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED EVENT HANDLERS" in line:
            result.append(line)
            result.extend(event_handlers)
            in_block = True
            continue

        if "// END EVENT HANDLERS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED DELAY OFFSETS" in line:
            result.append(line)
            result.extend(delay_offsets)
            in_block = True
            continue

        if "// END GENERATED DELAY OFFSETS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED VOLTAGE OFFSETS" in line:
            result.append(line)
            result.extend(voltage_offsets_generated)
            in_block = True
            continue

        if "// END GENERATED VOLTAGE OFFSETS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if "// GENERATED CONFIG DELAY OFFSETS" in line:
            result.append(line)
            result.extend(config_delay_offsets)
            in_block = True
            continue

        if "// END GENERATED CONFIG DELAY OFFSETS" in line:
            in_block = False
            result.append("\n" + line)
            continue

        if not in_block:
            result.append(line)

    with open(srcpath, "w") as file:
        file.writelines(result)



# clean the source file from the generated components
def clean_src(src_file):
    with open(src_file, "r") as file:
        lines = file.readlines()
    result = []
    in_block = False
    for line in lines:
        if "END" in line:
            result.append(line)
            in_block = False
            continue

        if "GENERATED" in line:
            result.append(line)
            in_block = True
            continue

        if not in_block:
            result.append(line)
            continue
    with open(src_file, "w") as file:
        file.writelines(result)


class PowerManager(gsys.Component):
    def __init__(
        self,
        parent: gsys.Component,
        name: str,
        schedule=False,
        schedule_file="attributes.json",
        component_list=None
    ):
        super().__init__(parent, name)
        src_file = self.get_file_path("power_manager.cpp")
        if component_list is None:
            self.component_list = list(parent.components.keys())
            self.component_list.remove(name)
        else:
            self.component_list = component_list
        print("detected components: ", self.component_list)

        clean_src(src_file)

        add_ports(self.component_list, src_file)

        self.add_sources(["power_manager.cpp"])

    def i_INPUT_STATE(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, "state_ctrl", signature="io")

    def i_INPUT_VOLTAGE(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, "voltage_ctrl", signature="io")

    def i_POWER_REPORT(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, "power_report", signature="io")
    
    def i_DELAY_CONFIG(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, "delay_config", signature="io")
