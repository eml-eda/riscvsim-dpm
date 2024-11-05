#include "../pm_addr.h"
#include "pmsis.h"

#define pm_state 0x20004000
#define pm_voltage 0x20005000
#define pm_config_delay_state 0x20007000
#define pm_report 0x20006000
#define pm_config_delay_voltage 0x20008000

//define voltage delays configurations
#define delay_on_idle 400000000
#define delay_idle_on 400000000
#define delay_on_sleep 1000000000
#define delay_sleep_on 4000000000

/**
 * @brief Transition the host from run state to idle state.
 */
void run_to_idle();

/**
 * @brief Transition the host from idle state to run state.
 */
void idle_to_run();

/**
 * @brief Transition the host from sleep state to run state.
 */
void sleep_to_run();

/**
 * @brief Start capturing power consumption.
 */
void capture_start();

/**
 * @brief Stop capturing power consumption.
 */
void capture_stop();

/**
 * @brief Get the captured power consumption of the system.
 * 
 * @return The power consumption value as double.
 */
double get_power_consumption();

/**
 * @brief Transition the host from run state to sleep state.
 */
void run_to_sleep();

/**
 * @brief Set the state of host to on.
 */
void switch_on();

/**
 * @brief Set the state of host to off.
 */
void switch_off();

/**
 * @brief Set the state of host to clock gated.
 */
void switch_clock_gate();

/**
 * @brief Configure the delays for state transitions.
 * 
 * @param on_off Delay for transitioning from on to off state.
 * @param off_on Delay for transitioning from off to on state.
 * @param on_cg Delay for transitioning from on to clock gate state.
 * @param cg_on Delay for transitioning from clock gate to on state.
 */
void config_state_delays(int on_off, int off_on, int on_cg, int cg_on);