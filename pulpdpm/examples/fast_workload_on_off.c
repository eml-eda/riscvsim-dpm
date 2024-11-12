#include <stdio.h>
#include <stdint.h>
#include "../pm_addr.h"
#include "pmsis.h"
#include "pm_functions.h"

extern const int delay_sleep_on_us;
int main()
{
    config_state_delays(delay_on_sleep, delay_sleep_on, delay_on_idle, delay_idle_on);
    switch_on();
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
        switch_off();
        pi_time_wait_us(1000); // 1 ms delay
        switch_on();
        // switch on delay
        pi_time_wait_us(delay_sleep_on_us);
    }
    capture_stop();
    printf("Average consumption: %f\n", get_power_consumption());

    pi_time_wait_us(100);
    return 0;
}
