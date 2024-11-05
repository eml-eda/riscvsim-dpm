#include "pm_functions.h"

volatile int *pm_state_ptr = (volatile int *)pm_state;
volatile float *pm_voltage_ptr = (volatile float *)pm_voltage;
volatile int *pm_report_ptr = (volatile int *)pm_report;
volatile int *pm_config_delay_states_ptr = (volatile int *)pm_config_delay_state;
volatile int *pm_config_delay_voltage_ptr = (volatile int *)pm_config_delay_voltage;
const int delay_idle_on_us = delay_idle_on / 1000000;
const int delay_sleep_on_us = delay_sleep_on / 1000000;

void run_to_idle()
{
    *(pm_config_delay_voltage_ptr) = delay_on_idle;
    *(pm_voltage_ptr + host_offset) = 0.8;
};

void idle_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_idle_on;
    *(pm_voltage_ptr + host_offset) = 1.2;
    // exit time
    pi_time_wait_us(delay_idle_on_us);
};

void sleep_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_sleep_on;
    *(pm_voltage_ptr + host_offset) = 1.2;
    // exit time
    pi_time_wait_us(delay_sleep_on_us);
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
    *(pm_voltage_ptr + host_offset) = 1.2;
    *(pm_state_ptr + host_offset) = on;
}

void switch_off()
{
    *(pm_state_ptr + host_offset) = off;
}

void switch_clock_gate()
{
    *(pm_state_ptr + host_offset) = on_clock_gated;
}

void config_state_delays(int on_off, int off_on, int on_cg, int cg_on)
{
    *(pm_config_delay_states_ptr + host_config_offset + on_off_offset) = on_off;
    *(pm_config_delay_states_ptr + host_config_offset + off_on_offset) = off_on;
    *(pm_config_delay_states_ptr + host_config_offset + cg_on_offset) = cg_on;
    *(pm_config_delay_states_ptr + host_config_offset + on_cg_offset) = on_cg;
}
