#include "../pm_addr.h"
#include "pmsis.h"

#define pm_state 0x20004000
#define pm_voltage 0x20005000
#define pm_config_delay_state 0x20007000
#define pm_report 0x20006000
#define pm_config_delay_voltage 0x20008000

void run_to_idle();
void idle_to_run();
void sleep_to_run();
void capture_start();
void capture_stop();
double get_power_consumption();
void run_to_sleep();