#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "../pm_addr.h"
#include "pmsis.h"

#define pm_state 0x20004000
#define pm_voltage 0x20005000
#define pm_config 0x20007000
#define pm_report 0x20006000

int main()
{
    volatile int *pm_state_ptr = (volatile int *)pm_state;
    volatile float *pm_voltage_ptr = (volatile float *)pm_voltage;
    volatile int *pm_report_ptr = (volatile int *)pm_report;
    volatile int *pm_config_delay = (volatile int *)pm_config;

    // configure delays
    *(pm_config_delay + host_config_offset + on_off_offset) = 50000;
    *(pm_config_delay + host_config_offset + off_on_offset) = 20000;
    *(pm_config_delay + host_config_offset + cg_on_offset) = 30000;
    *(pm_config_delay + host_config_offset + on_cg_offset) = 40000;

    // read sensor
    int data1, data2, data3;
    
    float current_voltage = 1.2;
    for (int i = 0; i < 10; i++)
    {
        *(pm_report_ptr) = start_capture;
        *(pm_state_ptr + host_offset) = on;
        data1 = *((uint32_t *)0x20000000);
        data2 = *((uint32_t *)0x20000100);
        data3 = *((uint32_t *)0x20000200);

        printf("data from sensor 1: %x\n", data1);
        printf("data from sensor 2: %x\n", data2);
        printf("data from sensor 3: %x\n\n", data3);

        // compute something
        double squared_sum = 0.0;
        squared_sum = data1 * data1 + data2 * data2 + data3 * data3;

        printf("mean squared : %f\n\n", squared_sum / 3);
        
        // go to sleep
        *(pm_report_ptr) = stop_capture;
        printf("power on consumption: %f\n", *(double *)(pm_report));
        *(pm_state_ptr + host_offset) = on_clock_gated;
        *(pm_report_ptr) = start_capture;
        pi_time_wait_us(500);
        *(pm_report_ptr) = stop_capture;
        printf("power on_cg consumption: %f\n", *(double *)(pm_report));

        // decrease voltage
        current_voltage-=0.1;
        *(pm_voltage_ptr + host_offset) = current_voltage;
    }
    return 0;
}
