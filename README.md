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

### pulp compiler and sdk


## Installing GVSoC

GVSoC can be installed by following the instruction found in [repository](https://github.com/eml-eda/gvsoc). To automate the process a script has been wrote to clone and install the tool automatically (clone_and_install_gvsoc.sh). The bin (_gvsoc/bin_) folder containing gvsoc executable has to be added to PATH environment variable.

## Repository structure

~~~bash
├── dpm
├── pulpdpm
│   └── app
└── tutorials
    ├── 0_how_to_build_a_system_from_scratch
    ├── 14_how_to_add_power_traces_to_a_component
    ├── 1_how_to_write_a_component_from_scratch
    ├── 2_how_to_make_components_communicate_together
    ├── 6_how_to_add_timing
    └── utils
~~~

This repository contains relevant tutorials (tutorials folder), together with a short guide on how to work with GVSoC (_GVSOC_guide.md_) and two example system, where the power_manager component is able to modify the power state of the component acting on the main power port:

- dpm: contains a system with a _ri5cy_ core.
- pulpdpm: contains a system with _pulp open board_ chip.

In both cases the Power Manager component is able to change the power state of the components by operating on the memory mapped registers directly from the firmware runnning on the simulated core.

## Description of the Power manager component

The Python generator of the component is "power_manager.py".
The PowerManager constructor can take a list of string, containing the names of the components to control. This list is used to generate the necessary connection ports, interfaces and source code to manage the changing of power state and voltages of the connected components. Alternatively, if no list is specified, it generates connections for all the components present in the same hierarchy level.

~~~Python
pm = power_manager.PowerManager(self, "pm", component_list=["host", "sensor1", "sensor2", "sensor3"])
~~~


The internal registers of the component are controlled by reading and writing its memory mapped ports. The available ports are:

- __i_INPUT_STATE()__: Writing to this port, can change the power state of the component: each component is assigned to an offset.
- __i_INPUT_VOLTAGE()__: Writing to this port, can change the voltage of the component: each component is assigned to an offset.
- __i_POWER_REPORT()__: Writing to this port, it is possible to start and stop recording power consumption. Reading from this port returns the last power consumption value.
- __i_DELAY_CONFIG()__: Writing to this port it is possible to specify the delays of each transition for each component. At each component is assigned a component offset, and at each component offset is assigned a transition offset for every direction of the power state change (4 possible transition for 3 states)

The component generates an header file (_pm_addr.h_) file containing the generated offsets, as in the following example:

~~~c
// defined states in power manager
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
#define host_offset 0
#define host_config_offset 0
#define sensor1_offset 1
#define sensor1_config_offset 4
#define sensor2_offset 2
#define sensor2_config_offset 8
#define sensor3_offset 3
#define sensor3_config_offset 12
~~~

The ports are then mapped in memory in the System component containing the all component and the instantiated power manager.

~~~Python
        ico.o_MAP(
            pm.i_INPUT_STATE(),
            "pm_state",
            base=0x20004000,
            size=0x00001000,
            rm_base=True,
        )

        ico.o_MAP(
            pm.i_INPUT_VOLTAGE(),
            "pm_voltage",
            base=0x20005000,
            size=0x00001000,
            rm_base=True,
        )
        
        ico.o_MAP(
            pm.i_POWER_REPORT(), 
            "pm_report",
            base=0x20006000,
            size=0x00000010,
            rm_base=True
        )

        ico.o_MAP(
            pm.i_DELAY_CONFIG(), 
            "pm_config",
            base=0x20007000,
            size=0x00001000,
            rm_base=True
        ) 
~~~

Therefore in the simulated binaries it is possible to control the power as in the following example.

~~~c
#define pm_state 0x20004000
#define pm_voltage 0x20005000
#define pm_config 0x20007000
#define pm_report 0x20006000

// inside main function
    ...
// definition of the pointers to the registers 
    volatile int *pm_state_ptr = (volatile int *)pm_state;
    volatile float *pm_voltage_ptr = (volatile float *)pm_voltage;
    volatile int *pm_report_ptr = (volatile int *)pm_report;
    volatile int *pm_config_delay = (volatile int *)pm_config;

// configure delay example, time in ps
    *(pm_config_delay + host_config_offset + on_off_offset) = 50000;
    *(pm_config_delay + host_config_offset + off_on_offset) = 20000;
    *(pm_config_delay + host_config_offset + cg_on_offset) = 30000;
    *(pm_config_delay + host_config_offset + on_cg_offset) = 40000;

// power measurement 
    *(pm_report_ptr) = start_capture;
    *(pm_report_ptr) = stop_capture;
    printf("power consumption: %f\n", *(double *)(pm_report));

// changing voltage
    *(pm_voltage_ptr + host_offset) = 0.9;
~~~

## Limitations of GVSoC for power modeling
If the description of the power source does not contain more than one voltage level, then changing voltage has no effect on power consumption, and the sole value is used for estimation, otherwise the consumption is computed depending on the applied voltage level, and the linear interpolation of the described values.
if the component to control, does not overload the _power_supply_set_ method, then the default function is called. This function has the same behavior both for _ON_ and _ON_CLOCK_GATED_ value, turning on the consumption of each power source component and its child component, meaning that both dynamic and leakage power is accounted. In the _OFF_  state instead, the default behavior is to turn off both power components.
