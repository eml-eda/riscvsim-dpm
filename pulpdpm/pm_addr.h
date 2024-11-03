// defined states in power manager
#define off 0x0
#define on_clock_gated 0x1
#define on 0x3

//define voltage delays configurations
#define delay_on_idle 400000000
#define delay_idle_on 400000000
#define delay_on_sleep 1000000000
#define delay_sleep_on 4000000000


// offset to control the power measurement
#define start_capture 0x1
#define stop_capture 0

//offsets of the config registers
#define on_off_offset 0
#define off_on_offset 1
#define on_cg_offset 2
#define cg_on_offset 3

//define pm addresses mapped to components
#define host_offset 0
#define host_config_offset 0
#define sensor1_offset 1
#define sensor1_config_offset 4
#define sensor2_offset 2
#define sensor2_config_offset 8
#define sensor3_offset 3
#define sensor3_config_offset 12
