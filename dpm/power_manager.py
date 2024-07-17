import gvsoc.systree as gsys 

class PowerManager(gsys.Component):
    def __init__(self, parent: gsys.Component, name: str):

        super().__init__(parent, name)
        self.add_sources(["power_manager.cpp"])
        self.add_properties


    def i_INPUT(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, 'input', signature='io')
    
    def o_POWER_CTRL(self, itf: gsys.SlaveItf):
        self.itf_bind('power_ctrl', itf, signature='wire<int>')

    def o_VOLTAGE_CTRL(self, itf: gsys.SlaveItf):
        self.itf_bind('voltage_ctrl', itf, signature='wire<int>')


    def o_POWER_CTRL_1(self, itf: gsys.SlaveItf):
        self.itf_bind('power_ctrl1', itf, signature='wire<int>')

    def o_VOLTAGE_CTRL_1(self, itf: gsys.SlaveItf):
        self.itf_bind('voltage_ctrl1', itf, signature='wire<int>')

    def o_POWER_CTRL_2(self, itf: gsys.SlaveItf):
        self.itf_bind('power_ctrl2', itf, signature='wire<int>')

    def o_VOLTAGE_CTRL_2(self, itf: gsys.SlaveItf):
        self.itf_bind('voltage_ctrl2', itf, signature='wire<int>')

    def o_POWER_CTRL_3(self, itf: gsys.SlaveItf):
        self.itf_bind('power_ctrl3', itf, signature='wire<int>')

    def o_VOLTAGE_CTRL_3(self, itf: gsys.SlaveItf):
        self.itf_bind('voltage_ctrl3', itf, signature='wire<int>')
