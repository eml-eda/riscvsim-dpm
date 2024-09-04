import gvsoc.systree as gsys 

def add_ports(component_list):
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



class PowerManager(gsys.Component):
    def __init__(self, parent: gsys.Component, name: str):
        super().__init__(parent, name)
        self.add_sources(["power_manager.cpp"])
        self.add_properties
        self.component_list = parent.components
        print("detected components: ", self.component_list )
        add_ports(self.component_list)
        
    def i_INPUT(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, 'input', signature='io')

