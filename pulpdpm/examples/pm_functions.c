#include "pm_functions.h"

void run_to_idle()
{
    *(pm_config_delay_voltage_ptr) = delay_on_idle;
    *(pm_voltage_ptr + host_offset) = 0.8;
};

void idle_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_idle_on;
    *(pm_voltage_ptr + host_offset) = 1.2;
};

void sleep_to_run()
{
    *(pm_config_delay_voltage_ptr) = delay_sleep_on;
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
