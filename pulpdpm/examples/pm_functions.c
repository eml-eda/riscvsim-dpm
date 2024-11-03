#include "pm_functions.h"

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
    pi_time_wait_us(delay_idle_on_us);
    *(pm_voltage_ptr + host_offset) = 1.2;
};

void sleep_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_sleep_on;
    // exit time
    pi_time_wait_us(delay_sleep_on_us);
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
