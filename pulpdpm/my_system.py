from pulp.chips.pulp_open.pulp_open_board import Pulp_open_board
import power_manager
import gvsoc.systree
import gvsoc.runner
import gvsoc.systree
import gvsoc.runner
from vp.clock_domain import Clock_domain

GAPY_TARGET = True

class PulpBoard(gvsoc.systree.Component):
    def __init__(self, parent, name, parser, options):
        super().__init__(parent, name, options=options)
        
        host= Pulp_open_board(self, "host", parser, options, use_ddr=False)
        
        # # connect the power control
        # # instantiate power manager
        pm = power_manager.PowerManager(self, "pm")
        soc_clock=Clock_domain(self, 'soc_clock_domain', frequency=50000000)
        soc_clock.o_CLOCK(pm.i_CLOCK())

        axi_ico = host.components['chip'].components['soc'].components['axi_ico']
        # axi_ico.add_mapping('pm_state', base=0x10500000,  size=0x10, rm_base= True)
        
        # power interconnect 
        # axi_ico.o_MAP(
        #     pm.i_INPUT_STATE(),
        #     "pm_state",
        #     base=0x20004000,
        #     size=0x00001000,
        #     rm_base=True,
        # )

        #connect power manager to pulp
        
        pm.o_POWER_CTRL_host(host.components['chip'].components['soc'].i_POWER())
        pm.o_VOLTAGE_CTRL_host(host.i_VOLTAGE())

      
# This is the top target that gapy will instantiate
class Target(gvsoc.runner.Target):
    def __init__(self, parser, options):
        super(Target, self).__init__(
            parser, options, model=PulpBoard, description="pulp virtual board"
        )