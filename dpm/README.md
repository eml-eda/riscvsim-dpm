# Power Manager Component

The Power manager component (power_manager) controls the power state of the various component at the same hierarchy level, following a schedule reported in a JSON file.

## Generation of the interconnection
In the `power_manager.py` python generator, the `power_manager.cpp` source file is updated with the definitions of the power and voltage ports that are connected to the components being controlled. This generation process applies to components at the same hierarchy level, that are also present in `attributes.json` file, which contains the scheduling times for each component.

## Generation of the scheduling
The scheduling is generated based on the information provided in the `attributes.json` file. The structure of the file is as follows:

```
{
    "period": <period in cc>,
    "components": {
        <component name>: {
            "activation_time": <cycle in the period at which the component activates>,
            "time_on": <duration of the activation, in cc>
        },
        <component name>: {
            "activation_time": <cycle in the period at which the component activates>,
            "time_on": <duration of the activation, in cc>
        },
        . . .
    }
}
```

All timing values are specified in clock cycles. The scheduling is generated in the source code (`power_manager.cpp`) by utilizing events. For each component, an event and its event handler are created. Then, in the `start_period_event_handler`, the events are scheduled according to the values specified in the JSON file.

Each handler toggles the value of the power state between ON and OFF. 