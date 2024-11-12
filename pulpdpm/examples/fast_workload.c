#include <stdio.h>
#include <stdint.h>
#include "../pm_addr.h"
#include "pmsis.h"
#include "pm_functions.h"

int main()
{
    // turn on the power consumption
    switch_on();
    // start recording power consumption
    capture_start();
    printf("Start workload\n");
    for (int j = 0; j < 100; j++)
    {
        double result = 0.0;
        for (int i = 0; i < 1000; i++)
        {
            result += i * i / (i + 2);
        }
        printf("Risultato del calcolo: %.2f\n", result);
        // go to lower consumption state during inactive period
        run_to_idle();
        pi_time_wait_us(1000); // 1 ms delay
        idle_to_run();
    }
    // stop recording energy consumption
    capture_stop();
    // get the recorded value from the Powermanager
    printf("Average consumption: %f\n", get_power_consumption());

    pi_time_wait_us(100);
    return 0;
}
