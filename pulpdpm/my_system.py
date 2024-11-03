from pulp.chips.pulp_open.pulp_open_board import Pulp_open_board
from vp.clock_domain import Clock_domain
import power_manager
import gvsoc.systree
import gvsoc.runner
import gvsoc.systree
import gvsoc.runner
import interco.router
import interco.router_proxy
import my_sensors

GAPY_TARGET = True

class PulpBoard(gvsoc.systree.Component):
    def __init__(self, parent, name, parser, options):
        super().__init__(parent, name, options=options)
        
        host= Pulp_open_board(self, "host", parser, options, use_ddr=False)
        
        # # connect the power control
        # # instantiate power manager
        soc_clock=Clock_domain(self, 'soc_clock_domain', frequency=50000000)

        ico = interco.router.Router(self, 'ico')
        axi_pm = interco.router_proxy.Router_proxy(self, 'axi_pm')
        
        self.bind(axi_pm, 'out', ico, 'input')
        
        sensor1 = my_sensors.GenericSensor(self, "sensor1")
        sensor2 = my_sensors.GenericSensor(self, "sensor2")
        sensor3 = my_sensors.GenericSensor(self, "sensor3")

        soc_clock.o_CLOCK(axi_pm.i_CLOCK())
        soc_clock.o_CLOCK(ico.i_CLOCK())
        soc_clock.o_CLOCK(sensor1.i_CLOCK())
        soc_clock.o_CLOCK(sensor2.i_CLOCK())
        soc_clock.o_CLOCK(sensor3.i_CLOCK())
        
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
        pm = power_manager.PowerManager(self, "pm", component_list=["host", "sensor1", "sensor2", "sensor3"])
        soc_clock.o_CLOCK(pm.i_CLOCK())

        #connect power manager to pulp
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
            pm.i_DELAY_STATE_CONFIG(), 
            "pm_state_delay_config",
            base=0x20007000,
            size=0x00001000,
            rm_base=True
        )
        
        ico.o_MAP(
            pm.i_DELAY_VOLTAGE_CONFIG(), 
            "pm_voltage_delay_config",
            base=0x20008000,
            size=0x00000100,
            rm_base=True
        )
        pm.o_POWER_CTRL_host(host.i_POWER())
        pm.o_VOLTAGE_CTRL_host(host.i_VOLTAGE())
      
# This is the top target that gapy will instantiate
class Target(gvsoc.runner.Target):
    def __init__(self, parser, options):
        super(Target, self).__init__(
            parser, options, model=PulpBoard, description="pulp virtual board"
        )