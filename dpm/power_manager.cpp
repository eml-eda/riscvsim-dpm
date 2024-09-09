#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>

// GENERATED SCHEDULING TIMES
	#define period 200
	#define active_time_host 30
	#define start_time_host 0
	#define active_time_sensor1 30
	#define start_time_sensor1 0
	#define active_time_sensor2 6
	#define start_time_sensor2 0
	#define active_time_sensor3 6
	#define start_time_sensor3 0
// END SCHEDULING TIMES

using namespace vp;

class PowerManager : public Component
{

public:
    PowerManager(ComponentConf &config);

    void reset(bool active);
    void start();

private:
    void handle_notif(vp::Block *__this, bool value);
    IoSlave input_itf;
    static void handle_event(Block *__this, ClockEvent *event);
    Trace trace;
    vp::ClockEvent start_period;

    // GENERATED EVENTS 

	vp::ClockEvent start_host;
	vp::ClockEvent start_sensor1;
	vp::ClockEvent start_sensor2;
	vp::ClockEvent start_sensor3;
    // END GENERATED EVENTS

    static void start_period_handler(vp::Block *__this, vp::ClockEvent *event);

    // GENERATED EVENT HANDLER DEFINITIONS

	static void start_host_handler(vp::Block *__this, vp::ClockEvent *event);
	static void start_sensor1_handler(vp::Block *__this, vp::ClockEvent *event);
	static void start_sensor2_handler(vp::Block *__this, vp::ClockEvent *event);
	static void start_sensor3_handler(vp::Block *__this, vp::ClockEvent *event);
    // END EVENT HANDLER DEFNITIONS

    // GENERATED INTERFACES

	WireMaster<int> power_ctrl_itf_host;
	WireMaster<int> voltage_ctrl_itf_host;
	WireMaster<int> power_ctrl_itf_sensor1;
	WireMaster<int> voltage_ctrl_itf_sensor1;
	WireMaster<int> power_ctrl_itf_sensor2;
	WireMaster<int> voltage_ctrl_itf_sensor2;
	WireMaster<int> power_ctrl_itf_sensor3;
	WireMaster<int> voltage_ctrl_itf_sensor3;
    // END INTERFACES
};

PowerManager::PowerManager(ComponentConf &config)
    : Component(config),
    start_period(this, start_period_handler) 
    // GENERATED EVENT CONSTRUCTORS
,
	start_host(this, PowerManager::start_host_handler),
	start_sensor1(this, PowerManager::start_sensor1_handler),
	start_sensor2(this, PowerManager::start_sensor2_handler),
	start_sensor3(this, PowerManager::start_sensor3_handler)
    // END EVENT CONSTRUCTORS
{
    this->traces.new_trace("trace", &this->trace);

    this->new_slave_port("input", &this->input_itf);


    // GENERATED POWER AND VOLTAGE PORTS

	this->new_master_port("power_ctrl_host", &this->power_ctrl_itf_host);
	this->new_master_port("voltage_ctrl_host", &this->voltage_ctrl_itf_host);
	this->new_master_port("power_ctrl_sensor1", &this->power_ctrl_itf_sensor1);
	this->new_master_port("voltage_ctrl_sensor1", &this->voltage_ctrl_itf_sensor1);
	this->new_master_port("power_ctrl_sensor2", &this->power_ctrl_itf_sensor2);
	this->new_master_port("voltage_ctrl_sensor2", &this->voltage_ctrl_itf_sensor2);
	this->new_master_port("power_ctrl_sensor3", &this->power_ctrl_itf_sensor3);
	this->new_master_port("voltage_ctrl_sensor3", &this->voltage_ctrl_itf_sensor3);
    // END GENERATED PORTS
}

void PowerManager::reset(bool active)
{
    if (active)
    {
        this->power_ctrl_itf_host.sync(PowerSupplyState::OFF);
    }
    else
        this->start_period.enqueue(1);
}

void PowerManager::start()
{
    this->start_period.enqueue(10);
}

void PowerManager::start_period_handler(vp::Block *__this, vp::ClockEvent *event)
{
    PowerManager *_this = (PowerManager *)__this;

    if (!_this->start_period.is_enqueued())
    {
        _this->trace.msg(vp::TraceLevel::INFO, "starting period");
        _this->start_period.enqueue(period);
        // GENERATED START EVENTS

		_this->start_host.enqueue(start_time_host); 
		_this->start_sensor1.enqueue(start_time_sensor1); 
		_this->start_sensor2.enqueue(start_time_sensor2); 
		_this->start_sensor3.enqueue(start_time_sensor3); 
        // END START EVENTS
    }
}

// GENERATED EVENT HANDLERS

void PowerManager::start_host_handler(vp::Block *__this, vp::ClockEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	static vp::PowerSupplyState current_state = OFF;
	if (!_this->start_host.is_enqueued())
	{
		// change power state
		current_state = current_state == OFF ? ON : OFF;
		_this->trace.msg(vp::TraceLevel::INFO, "changing status of host to " + current_state);
		_this->power_ctrl_itf_host.sync(current_state);
		if (current_state == ON)
			_this->start_host.enqueue(active_time_host);
	}
}
void PowerManager::start_sensor1_handler(vp::Block *__this, vp::ClockEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	static vp::PowerSupplyState current_state = OFF;
	if (!_this->start_sensor1.is_enqueued())
	{
		// change power state
		current_state = current_state == OFF ? ON : OFF;
		_this->trace.msg(vp::TraceLevel::INFO, "changing status of sensor1 to " + current_state);
		_this->power_ctrl_itf_sensor1.sync(current_state);
		if (current_state == ON)
			_this->start_sensor1.enqueue(active_time_sensor1);
	}
}
void PowerManager::start_sensor2_handler(vp::Block *__this, vp::ClockEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	static vp::PowerSupplyState current_state = OFF;
	if (!_this->start_sensor2.is_enqueued())
	{
		// change power state
		current_state = current_state == OFF ? ON : OFF;
		_this->trace.msg(vp::TraceLevel::INFO, "changing status of sensor2 to " + current_state);
		_this->power_ctrl_itf_sensor2.sync(current_state);
		if (current_state == ON)
			_this->start_sensor2.enqueue(active_time_sensor2);
	}
}
void PowerManager::start_sensor3_handler(vp::Block *__this, vp::ClockEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	static vp::PowerSupplyState current_state = OFF;
	if (!_this->start_sensor3.is_enqueued())
	{
		// change power state
		current_state = current_state == OFF ? ON : OFF;
		_this->trace.msg(vp::TraceLevel::INFO, "changing status of sensor3 to " + current_state);
		_this->power_ctrl_itf_sensor3.sync(current_state);
		if (current_state == ON)
			_this->start_sensor3.enqueue(active_time_sensor3);
	}
}
// END EVENT HANDLERS

extern "C" Component *gv_new(ComponentConf &config)
{
    return new PowerManager(config);
}
