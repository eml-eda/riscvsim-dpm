#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>
#include <string>
#include <iostream>
#include <queue>
#include <utility>

using namespace vp;

// typedef struct delay_holder
// {
// 	int on_off;
// 	int off_on;
// 	int on_cg;
// 	int cg_on;
// } delay_holder;

static char statename[3][15] = {"OFF", "ON", "ON CLOCK GATED"};
class PowerManager : public Component
{

public:
	PowerManager(ComponentConf &config);
	void reset(bool active);

private:
	static vp::IoReqStatus handle_state(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_voltage(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_power_report(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_delay_config(vp::Block *__this, vp::IoReq *req);
	IoSlave input_state_itf;
	IoSlave input_voltage_itf;
	IoSlave power_report_itf;
	IoSlave delay_config_itf;
	Trace trace;
	double last_power_measure;

	// GENERATED EVENTS
	TimeEvent delay_host;
	TimeEvent delay_sensor1;
	TimeEvent delay_sensor2;
	TimeEvent delay_sensor3;

	// END GENERATED EVENTS

	// GENERATED EVENT HANDLER DEFINITIONS
	static void host_delay_handler(vp::Block *__this, vp::TimeEvent *event);
	static void sensor1_delay_handler(vp::Block *__this, vp::TimeEvent *event);
	static void sensor2_delay_handler(vp::Block *__this, vp::TimeEvent *event);
	static void sensor3_delay_handler(vp::Block *__this, vp::TimeEvent *event);

	// END EVENT HANDLER DEFNITIONS

	// GENERATED INTERFACES
	WireMaster<int> power_ctrl_itf_host;
	WireMaster<double> voltage_ctrl_itf_host;
	WireMaster<int> power_ctrl_itf_sensor1;
	WireMaster<double> voltage_ctrl_itf_sensor1;
	WireMaster<int> power_ctrl_itf_sensor2;
	WireMaster<double> voltage_ctrl_itf_sensor2;
	WireMaster<int> power_ctrl_itf_sensor3;
	WireMaster<double> voltage_ctrl_itf_sensor3;

	// END INTERFACES

	// GENERATED DELAY REGISTERS
	unsigned int host_delays[4] = {1,1,1,1};
	int host_next_state;
	unsigned int sensor1_delays[4] = {1,1,1,1};
	int sensor1_next_state;
	unsigned int sensor2_delays[4] = {1,1,1,1};
	int sensor2_next_state;
	unsigned int sensor3_delays[4] = {1,1,1,1};
	int sensor3_next_state;

	// END DELAY REGISTERS

	// GENERATED VCD PS SIGNAL
	vp::Signal<int> host_state;
	vp::Signal<int> sensor1_state;
	vp::Signal<int> sensor2_state;
	vp::Signal<int> sensor3_state;

	// END GENERATED VCD PS SIGNAL
};

PowerManager::PowerManager(ComponentConf &config)
	: Component(config)
	  // GENERATED EVENT CONSTRUCTORS
	, delay_host(this, host_delay_handler), host_state(*this, "host_state", 3) 	, delay_sensor1(this, sensor1_delay_handler), sensor1_state(*this, "sensor1_state", 3) 	, delay_sensor2(this, sensor2_delay_handler), sensor2_state(*this, "sensor2_state", 3) 	, delay_sensor3(this, sensor3_delay_handler), sensor3_state(*this, "sensor3_state", 3) 
// END EVENT CONSTRUCTORS
{
	this->traces.new_trace("trace", &this->trace, vp::DEBUG);
	this->new_slave_port("state_ctrl", &this->input_state_itf);
	this->new_slave_port("voltage_ctrl", &this->input_voltage_itf);
	this->new_slave_port("power_report", &this->power_report_itf);
	this->new_slave_port("delay_config", &this->delay_config_itf);
	this->input_state_itf.set_req_meth(handle_state);
	this->input_voltage_itf.set_req_meth(handle_voltage);
	this->power_report_itf.set_req_meth(handle_power_report);
	this->delay_config_itf.set_req_meth(handle_delay_config);

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
}

// GENERATED EVENT HANDLERS
void PowerManager::host_delay_handler(vp::Block *__this, vp::TimeEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->power_ctrl_itf_host.sync(_this->host_next_state);
	_this->trace.msg(vp::TraceLevel::DEBUG, "switching power state of host to %s\n", statename[_this->host_next_state]);
_this->host_state.set(_this->host_next_state);
}
void PowerManager::sensor1_delay_handler(vp::Block *__this, vp::TimeEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->power_ctrl_itf_sensor1.sync(_this->sensor1_next_state);
	_this->trace.msg(vp::TraceLevel::DEBUG, "switching power state of sensor1 to %s\n", statename[_this->sensor1_next_state]);
_this->sensor1_state.set(_this->sensor1_next_state);
}
void PowerManager::sensor2_delay_handler(vp::Block *__this, vp::TimeEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->power_ctrl_itf_sensor2.sync(_this->sensor2_next_state);
	_this->trace.msg(vp::TraceLevel::DEBUG, "switching power state of sensor2 to %s\n", statename[_this->sensor2_next_state]);
_this->sensor2_state.set(_this->sensor2_next_state);
}
void PowerManager::sensor3_delay_handler(vp::Block *__this, vp::TimeEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->power_ctrl_itf_sensor3.sync(_this->sensor3_next_state);
	_this->trace.msg(vp::TraceLevel::DEBUG, "switching power state of sensor3 to %s\n", statename[_this->sensor3_next_state]);
_this->sensor3_state.set(_this->sensor3_next_state);
}

// END EVENT HANDLERS

vp::IoReqStatus PowerManager::handle_state(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received power state request at offset 0x%lx, size 0x%lx, is_write %d\n",
					 req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling power state request...\n");
		int reqstate = (*req->get_data()) & 3;
		int power_state;
		switch (reqstate)
		{
		case 0:
			power_state = OFF;
			break;
		case 1:
			power_state = ON_CLOCK_GATED;
			break;
		case 2:
			power_state = OFF;
			break;
		case 3:
			power_state = ON;
			break;
		default:
			break;
		}

		int addr = req->get_addr();

		int picoseconds = 0;
		switch (addr)
		{
		// GENERATED DELAY OFFSETS

        case 0:
			if (!_this->delay_host.is_enqueued())
			{
				_this->host_next_state = power_state;
				// if next state is on check previous state
				if (power_state == ON)
				{
					if (_this->host_state.get() == OFF)
						picoseconds = _this->host_delays[1];//off-on
					else
						picoseconds = _this->host_delays[3];//cg-on
				}
				else if (power_state == OFF)
				{
					picoseconds = _this->host_delays[0];//on-off
				}
				else
					picoseconds = _this->host_delays[2];//on-cg

				_this->delay_host.enqueue(picoseconds);
			}
			else
				_this->trace.msg(vp::TraceLevel::DEBUG, "Last change of host is still  in progress...\n");
			break;

        case 4:
			if (!_this->delay_sensor1.is_enqueued())
			{
				_this->sensor1_next_state = power_state;
				// if next state is on check previous state
				if (power_state == ON)
				{
					if (_this->sensor1_state.get() == OFF)
						picoseconds = _this->sensor1_delays[1];//off-on
					else
						picoseconds = _this->sensor1_delays[3];//cg-on
				}
				else if (power_state == OFF)
				{
					picoseconds = _this->sensor1_delays[0];//on-off
				}
				else
					picoseconds = _this->sensor1_delays[2];//on-cg

				_this->delay_sensor1.enqueue(picoseconds);
			}
			else
				_this->trace.msg(vp::TraceLevel::DEBUG, "Last change of sensor1 is still  in progress...\n");
			break;

        case 8:
			if (!_this->delay_sensor2.is_enqueued())
			{
				_this->sensor2_next_state = power_state;
				// if next state is on check previous state
				if (power_state == ON)
				{
					if (_this->sensor2_state.get() == OFF)
						picoseconds = _this->sensor2_delays[1];//off-on
					else
						picoseconds = _this->sensor2_delays[3];//cg-on
				}
				else if (power_state == OFF)
				{
					picoseconds = _this->sensor2_delays[0];//on-off
				}
				else
					picoseconds = _this->sensor2_delays[2];//on-cg

				_this->delay_sensor2.enqueue(picoseconds);
			}
			else
				_this->trace.msg(vp::TraceLevel::DEBUG, "Last change of sensor2 is still  in progress...\n");
			break;

        case 12:
			if (!_this->delay_sensor3.is_enqueued())
			{
				_this->sensor3_next_state = power_state;
				// if next state is on check previous state
				if (power_state == ON)
				{
					if (_this->sensor3_state.get() == OFF)
						picoseconds = _this->sensor3_delays[1];//off-on
					else
						picoseconds = _this->sensor3_delays[3];//cg-on
				}
				else if (power_state == OFF)
				{
					picoseconds = _this->sensor3_delays[0];//on-off
				}
				else
					picoseconds = _this->sensor3_delays[2];//on-cg

				_this->delay_sensor3.enqueue(picoseconds);
			}
			else
				_this->trace.msg(vp::TraceLevel::DEBUG, "Last change of sensor3 is still  in progress...\n");
			break;

			// END GENERATED DELAY OFFSETS

		default:
			_this->trace.msg(vp::TraceLevel::DEBUG, "No component associated with offset %d\n", addr);
			break;
		}
	}
	return vp::IoReqStatus::IO_REQ_OK;
}

vp::IoReqStatus PowerManager::handle_voltage(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received voltage request at offset 0x%lx, size 0x%lx, is_write %d\n", req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		double voltage = (*(double*)req->get_data());
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling voltage request with %f...\n", voltage);

		int addr = req->get_addr();

		switch (addr)
		{
		// GENERATED VOLTAGE OFFSETS
		case 0:
			_this->voltage_ctrl_itf_host.sync(voltage);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of host to %f\n");
			break;
		case 4:
			_this->voltage_ctrl_itf_sensor1.sync(voltage);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of sensor1 to %f\n");
			break;
		case 8:
			_this->voltage_ctrl_itf_sensor2.sync(voltage);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of sensor2 to %f\n");
			break;
		case 12:
			_this->voltage_ctrl_itf_sensor3.sync(voltage);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of sensor3 to %f\n");
			break;

		// END GENERATED VOLTAGE OFFSETS
		default:
			_this->trace.msg(vp::TraceLevel::DEBUG, "No component associated with offset %d\n", addr);
			break;
		}
	}
	return vp::IoReqStatus::IO_REQ_OK;
}

vp::IoReqStatus PowerManager::handle_delay_config(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received delay config at offset 0x%lx, size 0x%lx, is_write %d\n", req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		int value = *(uint32_t*)req->get_data();
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling delay config request...%x\n", value);

		int addr = req->get_addr();

		switch (addr)
		{
		// GENERATED CONFIG DELAY OFFSETS

        case 0:
		case 4:
		case 8:
		case 12:
			_this->host_delays[(addr-0)/4] = value;
			_this->trace.msg(vp::TraceLevel::DEBUG, "New configuration is: on-off: %d, off-on: %d, on-cg: %d, cg-on: %d\n", _this->host_delays[0],_this->host_delays[1],_this->host_delays[2],_this->host_delays[3]);
		break;

        case 16:
		case 20:
		case 24:
		case 28:
			_this->sensor1_delays[(addr-16)/4] = value;
			_this->trace.msg(vp::TraceLevel::DEBUG, "New configuration is: on-off: %d, off-on: %d, on-cg: %d, cg-on: %d\n", _this->sensor1_delays[0],_this->sensor1_delays[1],_this->sensor1_delays[2],_this->sensor1_delays[3]);
		break;

        case 32:
		case 36:
		case 40:
		case 44:
			_this->sensor2_delays[(addr-32)/4] = value;
			_this->trace.msg(vp::TraceLevel::DEBUG, "New configuration is: on-off: %d, off-on: %d, on-cg: %d, cg-on: %d\n", _this->sensor2_delays[0],_this->sensor2_delays[1],_this->sensor2_delays[2],_this->sensor2_delays[3]);
		break;

        case 48:
		case 52:
		case 56:
		case 60:
			_this->sensor3_delays[(addr-48)/4] = value;
			_this->trace.msg(vp::TraceLevel::DEBUG, "New configuration is: on-off: %d, off-on: %d, on-cg: %d, cg-on: %d\n", _this->sensor3_delays[0],_this->sensor3_delays[1],_this->sensor3_delays[2],_this->sensor3_delays[3]);
		break;

		// END GENERATED CONFIG DELAY OFFSETS
		default:
			_this->trace.msg(vp::TraceLevel::DEBUG, "No component associated with offset %d\n", addr);
			break;
		}
	}
	return vp::IoReqStatus::IO_REQ_OK;
}

vp::IoReqStatus PowerManager::handle_power_report(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received report request at offset 0x%lx, size 0x%lx, is_write %d\n",
					 req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		double dynamic_power, static_power;
		int data = (*req->get_data()) & 1;

		if (data == 0)
		{
			_this->power.get_engine()->stop_capture();
			_this->last_power_measure = _this->power.get_engine()->get_average_power(dynamic_power, static_power);
			fprintf(stderr, "@power.measure_%ld@%f@\n", _this->time.get_time(), _this->last_power_measure);
		}
		else if (data == 1)
		{
			_this->power.get_engine()->start_capture();
		}
	}
	else
	{
		*(double *)req->get_data() = _this->last_power_measure;
		_this->trace.msg(vp::TraceLevel::DEBUG, "Returning %f\n", _this->last_power_measure);
	}

	return vp::IoReqStatus::IO_REQ_OK;
}

extern "C" Component *gv_new(ComponentConf &config)
{
	return new PowerManager(config);
}
