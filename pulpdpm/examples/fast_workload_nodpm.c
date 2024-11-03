#include <stdio.h>
#include <stdint.h>
#include "../pm_addr.h"
#include "pmsis.h"

#define pm_state 0x20004000
#define pm_voltage 0x20005000
#define pm_config_delay_state 0x20007000
#define pm_report 0x20006000
#define pm_config_delay_voltage 0x20008000

volatile int *pm_state_ptr = (volatile int *)pm_state;
volatile float *pm_voltage_ptr = (volatile float *)pm_voltage;
volatile int *pm_report_ptr = (volatile int *)pm_report;
volatile int *pm_config_delay_states_ptr = (volatile int *)pm_config_delay_state;
volatile int *pm_config_delay_voltage_ptr = (volatile int *)pm_config_delay_voltage;

void run_to_idle()
{
    *(pm_config_delay_voltage_ptr) = delay_on_idle;
    *(pm_voltage_ptr + host_offset) = 0.8;
};

void idle_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_idle_on;
    // exit time
    pi_time_wait_us(delay_idle_on / 1000);
    *(pm_voltage_ptr + host_offset) = 1.2;
};

void sleep_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_sleep_on;
    // exit time
    pi_time_wait_us(delay_sleep_on / 1000);
    *(pm_voltage_ptr + host_offset) = 1.2;
};

void capture_start()
{
    *(pm_report_ptr) = start_capture;
}

void capture_stop()
{
    *(pm_report_ptr) = stop_capture;
}

double get_power_consumption()
{
    return *(double *)(pm_report);
}

void run_to_sleep()
{
    *(pm_config_delay_voltage_ptr) = delay_on_sleep;
    *(pm_voltage_ptr + host_offset) = 0.2;
};

void switch_on()
{
    *(pm_state_ptr + host_offset) = on;
}

int main()
{
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
        pi_time_wait_us(1000); // 1 ms delay
    }
    capture_stop();
    printf("total consumption: %f\n", get_power_consumption());

    pi_time_wait_us(100);
    return 0;
}
