#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>
#include <string>
#include <iostream>
#include <queue>
#include <utility>

using namespace vp;

typedef struct comp_to_change
{
	float voltage;
	int address;
} comp_to_change;

static char statename[3][15] = {"OFF", "ON", "ON CLOCK GATED"};
class PowerManager : public Component
{

public:
	PowerManager(ComponentConf &config);

private:
	static void voltage_delay_handler(vp::Block *__this, vp::TimeEvent *event);
	static vp::IoReqStatus handle_state(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_voltage(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_power_report(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_state_delay_config(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_voltage_delay_config(vp::Block *__this, vp::IoReq *req);
	IoSlave input_state_itf;
	IoSlave input_voltage_itf;
	IoSlave power_report_itf;
	IoSlave state_delay_config_itf;
	IoSlave voltage_delay_config_itf;
	Trace trace;
	double last_power_measure;
	uint64_t delay_voltage_value = 1;
	comp_to_change to_change;

	TimeEvent delay_voltage;
	// GENERATED EVENTS

	// END GENERATED EVENTS

	// GENERATED EVENT HANDLER DEFINITIONS

	// END EVENT HANDLER DEFNITIONS

	// GENERATED INTERFACES

	// END INTERFACES

	// GENERATED DELAY REGISTERS

	// END DELAY REGISTERS

	// GENERATED VCD PS SIGNAL

	// END GENERATED VCD PS SIGNAL
};

PowerManager::PowerManager(ComponentConf &config)
	: Component(config), delay_voltage(this, voltage_delay_handler)
	  // GENERATED EVENT CONSTRUCTORS

// END EVENT CONSTRUCTORS
{
	this->traces.new_trace("trace", &this->trace, vp::DEBUG);
	this->new_slave_port("state_ctrl", &this->input_state_itf);
	this->new_slave_port("voltage_ctrl", &this->input_voltage_itf);
	this->new_slave_port("power_report", &this->power_report_itf);
	this->new_slave_port("state_delay_config", &this->state_delay_config_itf);
	this->new_slave_port("voltage_delay_config", &this->voltage_delay_config_itf);
	this->input_state_itf.set_req_meth(handle_state);
	this->input_voltage_itf.set_req_meth(handle_voltage);
	this->power_report_itf.set_req_meth(handle_power_report);
	this->state_delay_config_itf.set_req_meth(handle_state_delay_config);
	this->voltage_delay_config_itf.set_req_meth(handle_voltage_delay_config);

	// GENERATED POWER AND VOLTAGE PORTS

	// END GENERATED PORTS
}


// GENERATED EVENT HANDLERS

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

		unsigned int picoseconds = 0;
		switch (addr)
		{
			// GENERATED DELAY OFFSETS

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
		float voltage = (*(float *)req->get_data());
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling voltage request with %f...\n", voltage);

		_this->to_change.address = req->get_addr();
		_this->to_change.voltage = voltage;

		if (!_this->delay_voltage.is_enqueued()){
			_this->delay_voltage.enqueue(_this->delay_voltage_value);
		}else
		_this->trace.msg(vp::TraceLevel::DEBUG, "Request ignored, another voltage request is in progress....\n");		
	}

	return vp::IoReqStatus::IO_REQ_OK;
}

vp::IoReqStatus PowerManager::handle_state_delay_config(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received delay config at offset 0x%lx, size 0x%lx, is_write %d\n", req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		int value = *(uint32_t *)req->get_data();
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling delay config request...%x\n", value);

		int addr = req->get_addr();

		switch (addr)
		{
			// GENERATED CONFIG DELAY OFFSETS

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

void PowerManager::voltage_delay_handler(vp::Block *__this, vp::TimeEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;

	switch (_this->to_change.address)
	{
	// GENERATED VOLTAGE OFFSETS

	// END GENERATED VOLTAGE OFFSETS
	default:
		_this->trace.msg(vp::TraceLevel::DEBUG, "No component associated with offset %d\n", _this->to_change.address);
		break;
	}
}

 vp::IoReqStatus PowerManager::handle_voltage_delay_config(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->delay_voltage_value = *((uint32_t *)req->get_data());
	_this->trace.msg(vp::TraceLevel::DEBUG, "delay of voltage change set to  %d\n", _this->delay_voltage_value);
	return vp::IoReqStatus::IO_REQ_OK;
}

extern "C" Component *gv_new(ComponentConf &config)
{
	return new PowerManager(config);
}
