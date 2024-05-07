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
