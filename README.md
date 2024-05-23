# Implementation of Dynamic Power Management Policies in a RISC-V Instruction Set Simulator

## Prerequisite

### ricv gnu compiler toolchain

The  GVSoC software need the riscv version of gcc compile, the following code installs both 32 and 64 bits version

~~~shell
git clone --recursive https://github.com/riscv/riscv-gnu-toolchain

sudo apt-get install autoconf automake autotools-dev curl python3 python3-pip libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev libslirp-dev

cd riscv-gnu-toolchain

./configure --prefix=/opt/riscv --enable-multilib

make 
~~~

## Installing GVSoC

GVSoC can be installed by following the instruction found in [repository](https://github.com/gvsoc/gvsoc). To automate the process a script has been wrote to clone and install the tool automatically (clone_and_install_gvsoc.sh)

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

In this example the **Rv64** class instantiate a clock component and the actual system (_SoC_), consequently it connects the output port of the clock with the input clock port of the _soc_  object, which code is:

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

Taking as an example the previous system code, in order to connect the new component, a few lines have to be added in the constructor:

~~~python
    comp = my_comp.MyComp(self, 'my_comp', value=0x12345678)
    ico.o_MAP(comp.i_INPUT(), 'comp', base=0x20000000, size=0x00001000, rm_base=True)
~~~

In these lines, the class MyComp is instantiated with a defined value, then its port is mapped by the interconnect object to a defined address that will be accessed by the binary to be executed.

### Power Modeling

Power modeling is based on power sources.
The power state of a component  in this example, is controlled on two ports:

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

* the first consinsts of 2 bit: one stating if the component is active, the second to activet the clock of the component.
* the second holds the voltage value.

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

As shown in the code example the power state of the component is set by calling the sync function of each port.
The definition of the power metrics of the two sources are described in the Python generator.
They consists in meaured power consumption at different voltage level, temperature and frequency.

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

This example models the power for any frequency and temperature.
The framework interpolates the power values based on the given parametrics.
In order to take care of the clock gating the _power_supply_method_ has to be overloaded.

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

The overloaded function just turn on the dynamic power where as soon as the power state is on.
To model the power of the acceses ( e.g. access to the memory or instruction) the framework relies on energy quantum that are interpolated depending on the voltage. This quantity needs only to be assign through the instruction:

~~~cpp
_this->access_power.account_energy_quantum();
~~~

Finally the power ports are added to the python generator of the component and bind into the system.

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

The simulated binary (main.c) go through the possible states and outputs the power consumption of the system.

## About Power in gvsoc

The documentation provided in the repository does't state or explain how power is handled in the simulator. In these section there might be incorrect, or not exhaiutive information.

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
The paramers are stated in the _values_ object. In this object for each temperature is indicated at each voltage, a certain frequency at which correspond a power consumption value.
In the example below, there is a definition of a power souce named "_background_power_": it has both dynamic and leakage power values and there are power metrics for a single temperature (25) and two voltages (600 and 1200) at _any_ frequencies.

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
