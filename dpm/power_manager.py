import gvsoc.systree as gsys 
import json

def add_ports( component_list, srcpath):
    interfaces_generated=""
    ports_generated=""
    #scans the component list and adds power and voltage port on the class
    for component in component_list:
            power_port_name = "o_POWER_CTRL_"+ component
            voltage_port_name = "o_VOLTAGE_CTRL_"+ component
            def power_ports(self, itf: gsys.SlaveItf, name=f'power_ctrl_{component}'):
                self.itf_bind(name, itf, signature='wire<int>')

            def voltage_ports(self, itf: gsys.SlaveItf, name=f'voltage_ctrl_{component}'):
                self.itf_bind(name, itf, signature='wire<int>')
            setattr(PowerManager, power_port_name, power_ports)
            setattr(PowerManager, voltage_port_name, voltage_ports)
            
    #port needs to be added also in the cpp file.

            interfaces_generated= interfaces_generated + f"\n\tWireMaster<int> power_ctrl_itf_{component};\n\tWireMaster<int> voltage_ctrl_itf_{component};"

            ports_generated = ports_generated + f"\n\tthis->new_master_port(\"power_ctrl_{component}\", &this->power_ctrl_itf_{component});\n\tthis->new_master_port(\"voltage_ctrl_{component}\", &this->voltage_ctrl_itf_{component});"

    
    with open(srcpath, 'r') as f:
        lines = f.readlines()
    result=[]

    in_block = False

    for line in lines:
        if "// GENERATED INTERFACES" in line:
            result.append(line)
            result.extend(interfaces_generated)
            in_block=True
            continue

        if "// END INTERFACES"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if "// GENERATED POWER AND VOLTAGE PORTS" in line:
            result.append(line)
            result.extend(ports_generated)
            in_block=True
            continue

        if "// END GENERATED PORTS"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if not in_block:
            result.append(line)

    with open(srcpath, 'w') as file:
        file.writelines(result)

   

def add_schedule(components, filepath, srcpath):
    events=""
    start_events=""
    event_handlers=""
    event_handlers_definitions=""
    event_constructors=""
    scheduling_times=""
    
    with open(filepath, "r") as file:
        schedule = json.load(file)

    scheduling_times=f"\t#define period {schedule["period"]}"
    for item in components:
        if item in schedule["components"]:
            scheduling_times=scheduling_times + f"\n\t#define active_time_{item} {schedule["components"][item]["time_on"]}\n\t#define start_time_{item} {schedule["components"][item]["activation_time"]}"
            
            events= events + f"\n\tvp::ClockEvent start_{item};"
            event_handlers_definitions= event_handlers_definitions +f"\n\tstatic void start_{item}_handler(vp::Block *__this, vp::ClockEvent *event);"

            event_constructors= event_constructors + f",\n\tstart_{item}(this, PowerManager::start_{item}_handler)"

            start_events= start_events +f"\n\t\t_this->start_{item}.enqueue(start_time_{item}); "

            event_handlers = event_handlers +f"\nvoid PowerManager::start_{item}_handler(vp::Block *__this, vp::ClockEvent *event)\n{{\n\tPowerManager *_this = (PowerManager *)__this;\n\tstatic vp::PowerSupplyState current_state = OFF;\n\tif (!_this->start_{item}.is_enqueued())\n\t{{\n\t\t// change power state\n\t\tcurrent_state = current_state == OFF ? ON : OFF;\n\t\t_this->trace.msg(vp::TraceLevel::INFO, \"changing status of {item} to \" + current_state);\n\t\t_this->power_ctrl_itf_{item}.sync(current_state);\n\t\tif (current_state == ON)\n\t\t\t_this->start_{item}.enqueue(active_time_{item});\n\t}}\n}}"

    # update the source file 
    
    with open(srcpath, 'r') as f:
        lines = f.readlines()
    result=[]

    in_block = False

    for line in lines:
        if "// GENERATED SCHEDULING TIMES" in line:
            result.append(line)
            result.extend(scheduling_times)
            in_block=True
            continue

        if "// END SCHEDULING TIMES"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if "// GENERATED EVENTS " in line:
            result.append(line)
            result.extend(events)
            in_block=True
            continue

        if "// END GENERATED EVENTS"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if "// GENERATED EVENT HANDLER DEFINITIONS" in line:
            result.append(line)
            result.extend(event_handlers_definitions)
            in_block=True
            continue

        if "// END EVENT HANDLER DEFNITIONS"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if "// GENERATED EVENT CONSTRUCTORS" in line:
            result.append(line)
            result.extend(event_constructors)
            in_block=True
            continue

        if "// END EVENT CONSTRUCTORS"  in line:
            in_block=False
            result.append("\n"+line)
            continue
        
        if "// GENERATED START EVENTS" in line:
            result.append(line)
            result.extend(start_events)
            in_block=True
            continue

        if "// END START EVENTS"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        if "// GENERATED EVENT HANDLERS" in line:
            result.append(line)
            result.extend(event_handlers)
            in_block=True
            continue

        if "// END EVENT HANDLERS"  in line:
            in_block=False
            result.append("\n"+line)
            continue

        


        if not in_block:
            result.append(line)

    with open(srcpath, 'w') as file:
        file.writelines(result)

    print("events:\n", events, "start events:\n", start_events, "event handlers:\n", event_handlers, "event handlers definitions:\n", event_handlers_definitions, "event constructors:\n", event_constructors, "cheduling times:\n ", scheduling_times)





class PowerManager(gsys.Component):
    def __init__(self, parent: gsys.Component, name: str):
        super().__init__(parent, name)
        schedule_file=self.get_file_path("attributes.json")
        src_file=self.get_file_path("power_manager.cpp")
        self.component_list = parent.components
        print("detected components: ", self.component_list.keys() )
        #parse schedule file
        with open(schedule_file, "r") as file:
            data = json.load(file)
        self.component_list = {k: v for k, v in self.component_list.items() if k in data["components"]}
        add_ports(self.component_list, src_file)
        add_schedule(self.component_list, schedule_file, src_file)
        self.add_sources(["power_manager.cpp"])

        
    def i_INPUT(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, 'input', signature='io')

