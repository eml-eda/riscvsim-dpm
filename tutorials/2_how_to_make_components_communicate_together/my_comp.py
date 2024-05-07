import gvsoc.systree
import gvsoc.systree as gsystree


class MyComp(gvsoc.systree.Component):
    def __init__(self, parent: gvsoc.systree.Component, name: str, value: int):
        super().__init__(parent, name)

        self.add_sources(["my_comp.cpp"])

        self.add_properties({"value": value})

    def i_INPUT(self) -> gvsoc.systree.SlaveItf:
        return gvsoc.systree.SlaveItf(self, "input", signature="io")

    # port to communicate with other component, syntax for output
    def o_NOTIF(self, itf: gvsoc.systree.SlaveItf):
        self.itf_bind("notif", itf, signature="wire<bool>")

    # this syntax is for input
    def i_RESULT(self) -> gvsoc.systree.SlaveItf:
        return gvsoc.systree.SlaveItf(self, "result", signature="wire<MyResult>")


class Comp2(gvsoc.systree.Component):
    def __init__(self, parent: gsystree.Component, name: str):
        super().__init__(parent, name)
        self.add_sources(["my_comp2.cpp"])

    # syntax for input
    def i_NOTIF(self) -> gsystree.SlaveItf:
        return gsystree.SlaveItf(self, "notif", signature="wire<bool>")

    # syntax for output
    def o_RESULT(self, itf: gsystree.SlaveItf):
        self.itf_bind("result", itf, signature="wire<MyResult>")
