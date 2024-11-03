#include <stdio.h>
#include <stdint.h>
#include "../pm_addr.h"
#include "pmsis.h"
#include "pm_functions.h"

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
        run_to_idle();
        pi_time_wait_us(1000); // 1 ms delay
        idle_to_run();
    }
    capture_stop();
    printf("total consumption: %f\n", get_power_consumption());

    pi_time_wait_us(100);
    return 0;
}
