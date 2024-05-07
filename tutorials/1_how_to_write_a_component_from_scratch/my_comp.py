import gvsoc.systree


class MyComp(gvsoc.systree.Component):
    # python generator for the component
    def __init__(self, parent: gvsoc.systree.Component, name: str, value: int):#parent and name are mandatory, value is optional
        super().__init__(parent, name)
        self.add_sources(['my_comp.cpp'])
        self.add_properties({#call the to add value parameter to the json configuration of the component
            "value": value
        })
    #input port, the name should correspont to c++ code
    def i_INPUT(self) -> gvsoc.systree.SlaveItf:
        return gvsoc.systree.SlaveItf(self, 'input', signature='io')