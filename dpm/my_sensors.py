import gvsoc.systree as gsys


class GenericSensor(gsys.Component):
    def __init__(self, parent: gsys.Component, name: str):
        super().__init__(parent, name)
        self.add_sources(["my_sensor.cpp"])

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

    def i_INPUT(self) -> gsys.SlaveItf:
        return gsys.SlaveItf(self, "input", signature="io")

