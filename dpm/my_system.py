import cpu.iss.riscv
import power_manager
import memory.memory
import vp.clock_domain
import interco.router
import utils.loader.loader
import gvsoc.systree
import gvsoc.runner
import my_sensors

GAPY_TARGET = True


class Soc(gvsoc.systree.Component):
    def __init__(self, parent, name, parser):
        super().__init__(parent, name)

        # Parse the arguments to get the path to the binary to be loaded
        [args, __] = parser.parse_known_args()

        binary = args.binary

        # Main interconnect
        ico = interco.router.Router(self, "ico")

        # Main memory
        mem = memory.memory.Memory(self, "mem", size=0x00100000)
        # The memory needs to be connected with a mpping. The rm_base is used to substract
        # the global address to the requests address so that the memory only gets a local offset.
        ico.o_MAP(mem.i_INPUT(), "mem", base=0x00000000, size=0x00100000, rm_base=True)

        # Instantiates the main core and connect fetch and data to the interconnect
        host = cpu.iss.riscv.Riscv(self, "host", isa="rv64imafdc")
        host.o_FETCH(ico.i_INPUT())
        host.o_DATA(ico.i_INPUT())

        host.add_properties({
            "power_models":{
                "background": {
                    "dynamic": {
                        "type": "linear",
                        "unit": "W",
                        "values": {
                            "25": {
                                "600.0": {
                                    "any": 0.00020
                                },
                                "1200.0": {
                                    "any": 0.00050
                                }
                            }
                        }
                    },
                    "leakage": {
                        "type": "linear",
                        "unit": "W",

                        "values": {
                            "25": {
                                "600.0": {
                                    "any": 0.00005
                                },
                                "1200.0": {
                                    "any": 0.00010
                                }
                            }
                        }
                    },
                },
                "access_power": {
                    "dynamic": {
                        "type": "linear",
                        "unit": "pJ",

                        "values": {
                            "25": {
                                "600.0": {
                                    "any": 5.00000
                                },
                                "1200.0": {
                                    "any": 10.00000
                                }
                            }
                        }
                    }
                }
            }})

        sensor1 = my_sensors.GenericSensor(self, "sensor1")
        sensor2 = my_sensors.GenericSensor(self, "sensor2")
        sensor3 = my_sensors.GenericSensor(self, "sensor3")
        
        ico.o_MAP(
            sensor1.i_INPUT(),
            "sensor1",
            base=0x20000000,
            size=0x00001000,
            rm_base=True,
            latency=100,
        )
        ico.o_MAP(
            sensor2.i_INPUT(),
            "sensor2",
            base=0x20000100,
            size=0x00001000,
            rm_base=True,
            latency=200,
        )
        ico.o_MAP(
            sensor3.i_INPUT(),
            "sensor3",
            base=0x20000200,
            size=0x00001000,
            rm_base=True,
            latency=300,
        )

        # connect the power control
        # instantiate power manager
        pm = power_manager.PowerManager(self, "pm")

        # power intercorrect
        pm.o_POWER_CTRL_host(host.i_POWER())
        pm.o_VOLTAGE_CTRL_host(host.i_VOLTAGE())

        pm.o_POWER_CTRL_sensor1(sensor1.i_POWER())
        pm.o_POWER_CTRL_sensor2(sensor2.i_POWER())
        pm.o_POWER_CTRL_sensor3(sensor3.i_POWER())

        pm.o_VOLTAGE_CTRL_sensor1(sensor1.i_VOLTAGE())
        pm.o_VOLTAGE_CTRL_sensor2(sensor2.i_VOLTAGE())
        pm.o_VOLTAGE_CTRL_sensor3(sensor3.i_VOLTAGE())

        # Finally connect an ELF loader, which will execute first and will then
        # send to the core the boot address and notify him he can start
        loader = utils.loader.loader.ElfLoader(self, "loader", binary=binary)
        loader.o_OUT(ico.i_INPUT())
        loader.o_START(host.i_FETCHEN())
        loader.o_ENTRY(host.i_ENTRY())

# This is a wrapping component of the real one in order to connect a clock generator to it
# so that it automatically propagate to other components
class Rv64(gvsoc.systree.Component):
    def __init__(self, parent, name, parser, options):
        super().__init__(parent, name, options=options)
        clock = vp.clock_domain.Clock_domain(self, "clock", frequency=100000000)
        soc = Soc(self, "soc", parser)
        clock.o_CLOCK(soc.i_CLOCK())
    


# This is the top target that gapy will instantiate
class Target(gvsoc.runner.Target):
    def __init__(self, parser, options):
        super(Target, self).__init__(
            parser, options, model=Rv64, description="RV64 virtual board"
        )
        



