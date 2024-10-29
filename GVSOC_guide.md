# Modeling components with GVSoC

## Build system and components

### Build a simple system

In order to build a system, _Python Generators_ are exploited to assemble the different components together. It is mandatory to declare the top level entity of our system as a target for the runner _**gapy**_, by inheriting the _gvsoc.runner.Target_ class and setting **GAPY_TARGET** to true:

~~~python
import gvsoc.runner

GAPY_TARGET= True

class Target(gvsoc.runner.Target):

    def __init__(self, parser, options):
        super(Target, self).__init__(parser, options,
            model=Rv64, description="RV64 virtual board")
~~~

The _model_ argument is the name of the class of the system that is going to be implemented.

All components, and also the system, are declared inheriting the _**gvsoc.systree.Component**_ class.

~~~python
class Rv64(gvsoc.systree.Component):

    def __init__(self, parent, name, parser, options):
        super().__init__(parent, name, options=options)
            
        clock = vp.clock_domain.Clock_domain(self, "clock", 
            frequency=100000000)
            
        soc = SoC(self, "soc", parser)
            
        clock.o_CLOCK(soc.i_CLOCK())
~~~

Note that the _parser_ option is not propagated but can be used to get or declare the command line arguments.

In this example the **Rv64** class instantiate a clock component and the actual system (_SoC_), consequently it connects the output port of the clock with the input clock port of the _Soc_  object. The clock is automatically propagated to the components included in _Soc_ class, which code is:

~~~python
class Soc(gvsoc.systree.Component):

    def __init__(self, parent, name, parser):
        super().__init__(parent, name)

        [args, __] = parser.parse_known_args()

        binary = args.binary

        # Main memory
        mem = memory.memory.Memory(self, "mem",     size=0x00100000)

        # Main interconnect
        ico = interco.router.Router(self, "ico")
        ico.o_MAP(mem.i_INPUT(), "mem", base=0x00000000, size=0x00100000, rm_base=True)

        # Instantiates the main core and connect fetch and
        # data to the interconnect
        host = cpu.iss.riscv.Riscv(self, "host", isa="rv64imafdc")
        host.o_FETCH(ico.i_INPUT())
        host.o_DATA(ico.i_INPUT())
        host.o_DATA_DEBUG(ico.i_INPUT())

        # ELF loader will execute first and will then 
        # send to the core the boot address and 
        # notify him he can start

        loader = utils.loader.loader.ElfLoader(self, "loader", binary=binary)

        loader.o_OUT(ico.i_INPUT())
        loader.o_START(host.i_FETCHEN())
        loader.o_ENTRY(host.i_ENTRY())

        gdbserver.gdbserver.Gdbserver(self, "gdbserver")
~~~

The SoC class consists of 3 main components:

* Host CPU: the riscv 64 bit core

* Memory: generic memory from the module library.
* Loader: the component responsible for loading the binary code to the core.
* Main interconnect: the component that handles the interconnection between the components. In this example, it maps the memory at base address 0x00000000.

the last line (_gdbserver_) is optional, and it permits to activate a gdb server in order to debug the execution of the firmware.

### Building a component

In order to describe a new component, it is needed to create a python script (_generator_) like following example:

~~~python

class MyComp(gvsoc.systree.Component):
    def __init__(self, parent: gvsoc.systree.Component, name: str, value: int):#parent and name are mandatory
        super().__init__(parent, name)
        self.add_sources(['my_comp.cpp'])
        self.add_properties({#add value parameter to the json configuration of the component
            "value": value
        })
    #input port, the name should correspond to c++ code
    def i_INPUT(self) -> gvsoc.systree.SlaveItf:
        return gvsoc.systree.SlaveItf(self, 'input', signature='io')
~~~

When defining a new component, it is mandatory to provide the constructor of its class with the parent and name argument. The cpp code associated to the component is linked by the _add_sources_ function. If the components holds some additional feature (e.g. it stores a value), it has to be added to the json configuration through the function _add_properties_. The behavior of the component is described through c++ code. The following code follows the python example:

~~~cpp
#include <vp/vp.hpp>
#include <vp/itf/io.hpp>

class MyComp : public vp::Component
{
public:
    MyComp(vp::ComponentConf &config);

private:
    static vp::IoReqStatus handle_req(vp::Block *__this, vp::IoReq *req);
    vp::IoSlave input_itf;
    uint32_t value;
};

MyComp::MyComp(vp::ComponentConf &config)
    : vp::Component(config)
{
    this->input_itf.set_req_meth(&MyComp::handle_req);
    this->new_slave_port("input", &this->input_itf);
    this->value = this->get_js_config()->get_child_int("value");
}

vp::IoReqStatus MyComp::handle_req(vp::Block *__this, vp::IoReq *req)
{
    MyComp *_this = (MyComp *)__this;

    printf("Received request at offset 0x%lx, size 0x%lx, is_write %d\n",
        req->get_addr(), req->get_size(), req->get_is_write());
    if (!req->get_is_write() && req->get_addr() == 0 && req->get_size() == 4)
    {
        *(uint32_t *)req->get_data() = _this->value;
    }
    return vp::IO_REQ_OK;
}


extern "C" vp::Component *gv_new(vp::ComponentConf &config)
{
    return new MyComp(config);
}

~~~

Let's break down the code and highlight the code features:

* The declared class (_MyComp_) inherits from the Component base class of the gvsoc library.
* Its constructor is call by the software passing the configuration
* Inside the constructor the interface is linked with the function _new_slave_port_ and its handler is connected with _set_req_meth_. The name passed as argument is the same as the one provided in the python method _i_INPUT_. Also, the configuration is retrieved from the python description with _get_js_config_ (in this example, the parameter _value_).
* The function _**gv_new**_ is needed by the framework in order to instantiate the class when the component is loaded.
* The handle of the interface port (_handle_req_) its called everytime a request must be handled. The method is necessarily static and it receives the instance of the class as argument.

Taking as an example the previous system code, in order to connect the new component, a few lines have to be added in the top system class:

~~~python
    comp = my_comp.MyComp(self, 'my_comp', value=0x12345678)
    ico.o_MAP(comp.i_INPUT(), 'comp', base=0x20000000, size=0x00001000, rm_base=True)
~~~

In these lines, the class MyComp is instantiated with a defined value, then its port is mapped by the interconnect object to a defined address that will be accessed by the binary to be executed.

The _Component_ object provides some basic connection port, which are consequently inherited by all the describred components:
    - i_RESET() : a boolean port to control the reset signal 
    - i_CLOCK() : a port to which a clock generator should be connecte
    - i_POWER() : a port that can be exploit to change the power state choosing from the available states.
    - i_VOLTAGE() : a port for controlling the component voltage (expressed as int)

### Power Modeling

Power modeling is based on power sources.
The following example component can control the power state of a connected component through two ports:

~~~cpp
//in class declaration
vp::WireMaster<int> power_ctrl_itf;
vp::WireMaster<int> voltage_ctrl_itf;
    ...
//in component contructor
this->new_master_port("power_ctrl", &this->power_ctrl_itf);
this->new_master_port("voltage_ctrl", &this->voltage_ctrl_itf);
~~~

In this example the two ports are controlled through registers:

* the first consists of 2 bit: the LSB stating if the component is active (i.e. consuming power), the other bit activates the clock of the component.
* the second register holds the voltage value.

~~~cpp
if (req->get_size() == 4){
    if (req->get_addr() == 0){
        if (req->get_is_write()){
            int power = (*(uint32_t *)req->get_data()) & 1;
            int clock = ((*(uint32_t *)req->get_data()) >> 1) & 1;
            int power_state;
            if (power){
                if (clock){
                    power_state = vp::PowerSupplyState::ON;
                } else {
                        power_state = vp::PowerSupplyState::ON_CLOCK_GATED;
                    }
            } else {
                    power_state = vp::PowerSupplyState::OFF;
            }
            _this->power_ctrl_itf.sync(power_state);
            }
        }
    else if (req->get_addr() == 4){
            if (req->get_is_write()){
                int voltage = *(uint32_t *)req->get_data();
                _this->voltage_ctrl_itf.sync(voltage);
            }
        }
    }
~~~

As shown in the code example the power state of the component is set by calling the sync function of each port, that will be connected to the _power_ and _voltage_ input ports of a second component.
The definition of the power metrics of the two sources are described in the Python generator.
They consists in measured power consumption at different voltage level, temperature and frequency.

~~~python
self.add_properties(
            {
                "background_power": {
                    "dynamic": {
                        "type": "linear",
                        "unit": "W",
                        "values": {
                            "25": {
                                "600.0": {"any": 0.00020},
                                "1200.0": {"any": 0.00050},
                            }
                        },
                    },
                    "leakage": {
                        "type": "linear",
                        "unit": "W",
                        "values": {
                            "25": {
                                "600.0": {"any": 0.00005},
                                "1200.0": {"any": 0.00010},
                            }
                        },
                    },
                },
                "access_power": {
                    "dynamic": {
                        "type": "linear",
                        "unit": "pJ",
                        "values": {
                            "25": {
                                "600.0": {"any": 5.00000},
                                "1200.0": {"any": 10.00000},
                            }
                        },
                    }
                },
            }
        )
~~~

This example models the power for any frequency, at temperature of 25 Celsius, of two different voltage levels.
The framework interpolates the power values based on the given parametrics.
In order to take care of the clock gating the _power_supply_set_ _callback_ method has to be overloaded, in the component which power state is controlled.

~~~cpp
void MyComp::power_supply_set(vp::PowerSupplyState state)
{
    if (state == vp::PowerSupplyState::ON)
    {
        this->background_power.dynamic_power_start();

    }
    else
    {
        this->background_power.dynamic_power_stop();
    }
}
~~~

The overloaded function just turn on the dynamic power counter as soon as the power state is on.
To model the power of the acceses ( e.g. access to the memory or instruction) the framework relies on energy quantum that are interpolated depending on the voltage. This quantity needs only to be assign through the instruction:

~~~cpp
_this->access_power.account_energy_quantum();
~~~

Finally the output power ports are added to the python generator of the component and bind to the second component into the system top python class.

~~~python
#in my_comp.py classes
def o_POWER_CTRL(self, itf: gvsoc.systree.SlaveItf):
    self.itf_bind('power_ctrl', itf, signature='wire<int>')

def o_VOLTAGE_CTRL(self, itf: gvsoc.systree.SlaveItf):
    self.itf_bind('voltage_ctrl', itf, signature='wire<int>')

#in my_system.py constructor
comp2 = my_comp.MyComp2(self, 'my_comp2')
ico.o_MAP(comp2.i_INPUT(), 'comp2', base=0x30000000, size=0x00001000, rm_base=True)
comp2.o_POWER_CTRL( comp.i_POWER  ())
comp2.o_VOLTAGE_CTRL( comp.i_VOLTAGE())
~~~

Note that every component inherited from _gvsoc.systree.Component_ is already provided with input power and voltage port (_i_POWER()_ an _i_VOLTAGE()_).

The simulated binary (Tutorial 14 -> main.c) go through the possible states and outputs the power consumption of the system.

~~~ c
int main()
{
   int voltages[] = { 600, 800, 1200};

for (int i=0; i<sizeof(voltages)/sizeof(int); i++)
{
    printf("Voltage %d\n", voltages[i]);

    *(volatile uint32_t *)0x30000004 = voltages[i];

    // Mesure power when off and no clock
    printf("OFF\n");
    *(volatile uint32_t *)0x10000000 = 0xabbaabba;
    *(volatile uint32_t *)0x10000000 = 0xdeadcaca;


    // Mesure power when on and no clock
    printf("ON clock-gated\n");
    *(volatile uint32_t *)0x30000000 = 1;
    *(volatile uint32_t *)0x10000000 = 0xabbaabba;
    *(volatile uint32_t *)0x10000000 = 0xdeadcaca;


    // Mesure power when on and clock
    printf("ON\n");
    *(volatile uint32_t *)0x30000000 = 0x3;
    *(volatile uint32_t *)0x10000000 = 0xabbaabba;
    *(volatile uint32_t *)0x10000000 = 0xdeadcaca;


    // Mesure power with accesses
    printf("ON with accesses\n");
    *(volatile uint32_t *)0x10000000 = 0xabbaabba;
    for (int i=0; i<20; i++)
    {
        *(volatile uint32_t *)0x20000000 = i;
    }
    *(volatile uint32_t *)0x10000000 = 0xdeadcaca;


    *(volatile uint32_t *)0x30000000 = 0;

    printf("\n\n");
}
}
~~~

In this example the address _0x10000000_ is mapped to a memory that acts as a power trigger: writing "0xabbaabba" starts capturing the consumed power, writing "0xdeadcaca" stops the capture and print the measured values on the trace logged into the terminal.

~~~bash
Voltage 600
OFF
@power.measure_0@0.000000@
ON clock-gated
@power.measure_1@0.000050@
ON
@power.measure_2@0.000250@
ON with accesses
@power.measure_3@0.000333@


Voltage 800
OFF
@power.measure_4@-0.000000@
ON clock-gated
@power.measure_5@0.000067@
ON
@power.measure_6@0.000367@
ON with accesses
@power.measure_7@0.000478@


Voltage 1200
OFF
@power.measure_8@0.000000@
ON clock-gated
@power.measure_9@0.000100@
ON
@power.measure_10@0.000600@
ON with accesses
@power.measure_11@0.000767@
~~~

## About Power in gvsoc

The documentation provided in the repository does't state or explain how power is handled in the simulator. In these section there might be incorrect, or not exhaustive information.

### Describing power sources and setting parameters
The simulator makes available 3 power states:

~~~cpp
#from vp/power/power.hpp
 enum PowerSupplyState
    {
        OFF=0,
        ON_CLOCK_GATED=2,
        ON=1
    };
~~~

The metrics of the power consumption are described through JSON. For each power source, there is a JSON object describing the dynamic and leakage power consumption.
In this object for each temperature is indicated at each voltage, a certain frequency at which correspond a power consumption value.
In the example below, there is a definition of a power source named "_background_power_": it has both dynamic and leakage power values and there are power metrics for a single temperature (25) and two voltages (600 and 1200) at _any_ frequencies.

~~~json
                "background_power": {
                    "dynamic": {
                        "type": "linear",
                        "unit": "W",
                        "values": {
                            "25": {
                                "600.0": {"any": 0.00020},
                                "1200.0": {"any": 0.00050},
                            }
                        },
                    },
                    "leakage": {
                        "type": "linear",
                        "unit": "W",
                        "values": {
                            "25": {
                                "600.0": {"any": 0.00005},
                                "1200.0": {"any": 0.00010},
                            }
                        },
                    },
                }
~~~

The change in power state and voltage, is immediate. In the standard core present in the models, changing the power state does not affect the execution of the code.
In the modeled chips present in the pulp directory, power is modeled on instructions and not on the running state of the component.  

## Timing in gvsoc
In the simulator is it possible to model timing and delay through clock events.

~~~ cpp
vp::ClockEvent event;
~~~

events needs to be configured in the costructor of the component:

~~~ cpp
MyComp::MyComp(vp::ComponentConf &config)
    : vp::Component(config), event(this, MyComp::handle_event)
{
~~~

The constructor of the event is call passing as parameter the component and the callback function that will be executed when the event is fired.

The main methods are:

* exec();
  * the event is executed immediately
* enqueue(int64_t cycles = 1);
  * the event is enqueued, and will be execute after a number of cycles
* cancel();
  * the execution of an enqueued event is cancelled.
* is_enqueued();
  * return true if the event is scheduled, false otherwise
* enable();
  * enable the event to be executed at each clock cycle
* disable();
  * disable the execution of the event at each clock cycle

Alternatively to clock event, TimeEvent class can be exploit with the same logic, instead of the number of clock periods, the event is scheduled with a time delay given in picoseconds.
 