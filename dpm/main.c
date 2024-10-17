#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pm_addr.h"


#define pm_state 0x20004000
#define pm_voltage 0x20005000

int main()
{
    volatile int *pm_state_ptr = (volatile int *)pm_state;
    volatile int *pm_voltage_ptr = (volatile int *)pm_voltage;
    // read sensor
    int data1, data2, data3;
    for (int i = 0; i < 10; i++)
    {
        *(pm_state_ptr + host_offset) = on;
        data1 = *((uint32_t *)0x20000000);
        data2 = *((uint32_t *)0x20000100);
        data3 = *((uint32_t *)0x20000200);

        printf("data from sensor 1: %x\n", data1);
        printf("data from sensor 2: %x\n", data2);
        printf("data from sensor 3: %x\n\n", data3);

        //compute something
        double squared_sum = 0.0;
        squared_sum = data1 * data1 + data2*data2 + data3*data3;

        printf("mean squared : %f\n\n", squared_sum/3);
        // go to sleep
        // busy wait
        *(pm_state_ptr + host_offset) = off;
        for (int i = 0; i < 500; i++)
        {
            i=i+1;
            i=i-1;
        }
    }
    return 0;
}
