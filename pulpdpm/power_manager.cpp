#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>
#include <string>
// GENERATED SCHEDULING TIMES
// END SCHEDULING TIMES

using namespace vp;
static char statename[3][15] = {"OFF", "ON", "ON CLOCK GATED"};
class PowerManager : public Component
{

public:
	PowerManager(ComponentConf &config);
	void reset(bool active);
#ifdef SCHEDULING
	void start();
#endif
private:
	static vp::IoReqStatus handle_state(vp::Block *__this, vp::IoReq *req);
	static vp::IoReqStatus handle_voltage(vp::Block *__this, vp::IoReq *req);
	IoSlave input_state_itf;
	IoSlave input_voltage_itf;
	Trace trace;
	vp::Signal<int> last_pm;

#ifdef SCHEDULING
	vp::ClockEvent start_period;

	// GENERATED EVENTS
	// END GENERATED EVENTS

	static void start_period_handler(vp::Block *__this, vp::ClockEvent *event);

	// GENERATED EVENT HANDLER DEFINITIONS
// END EVENT HANDLER DEFNITIONS
#endif
	// GENERATED INTERFACES

	WireMaster<int> power_ctrl_itf_host;
	WireMaster<int> voltage_ctrl_itf_host;
	// END INTERFACES
};

PowerManager::PowerManager(ComponentConf &config)
	: Component(config), last_pm(*this, "last_pm_received", 3)
#ifdef SCHEDULING
	  ,
	  start_period(this, start_period_handler)
// GENERATED EVENT CONSTRUCTORS
// END EVENT CONSTRUCTORS
#endif
{
	this->traces.new_trace("trace", &this->trace, vp::DEBUG);

	this->new_slave_port("state_ctrl", &this->input_state_itf);
	this->new_slave_port("voltage_ctrl", &this->input_voltage_itf);
	this->input_state_itf.set_req_meth(handle_state);
	this->input_voltage_itf.set_req_meth(handle_voltage);

	// GENERATED POWER AND VOLTAGE PORTS

	this->new_master_port("power_ctrl_host", &this->power_ctrl_itf_host);
	this->new_master_port("voltage_ctrl_host", &this->voltage_ctrl_itf_host);
	// END GENERATED PORTS
}

void PowerManager::reset(bool active)
{
	if (active)
	{
		this->power_ctrl_itf_host.sync(PowerSupplyState::OFF);
	}
#ifdef SCHEDULING
	else
		this->start_period.enqueue(1);
#endif
}
#ifdef SCHEDULING
void PowerManager::start()
{
	this->start_period.enqueue(10);
}

void PowerManager::start_period_handler(vp::Block *__this, vp::ClockEvent *event)
{
	PowerManager *_this = (PowerManager *)__this;

	if (!_this->start_period.is_enqueued())
	{
		_this->trace.msg(vp::TraceLevel::DEBUG, "starting period\n");
		_this->start_period.enqueue(period);
		_this->power.get_engine()->stop_capture();
		double dynamic_power, static_power;
		fprintf(stderr, "@power.measure_%d@%f@\n", _this->time.get_time(), _this->power.get_engine()->get_average_power(dynamic_power, static_power));
		_this->power.get_engine()->start_capture();
		// GENERATED START EVENTS
		// END START EVENTS
	}
}

// GENERATED EVENT HANDLERS
// END EVENT HANDLERS
#endif

vp::IoReqStatus PowerManager::handle_state(vp::Block *__this, vp::IoReq *req)
{
	PowerManager *_this = (PowerManager *)__this;
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received power state request at offset 0x%lx, size 0x%lx, is_write %d\n",
					 req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling power state request...\n");
		int reqstate = (*req->get_data()) & 3;
		_this->last_pm.set(reqstate);
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

		switch (addr)
		{
		// GENERATED POWER OFFSETS
		case 0:
			_this->power_ctrl_itf_host.sync(power_state);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching power state of host to %s\n", statename[power_state]);
			break;

		// END GENERATED POWER OFFSETS
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
	_this->trace.msg(vp::TraceLevel::DEBUG, "Received voltage request at offset 0x%lx, size 0x%lx, is_write %d\n",
					 req->get_addr(), req->get_size(), req->get_is_write());

	if (req->get_is_write())
	{
		_this->trace.msg(vp::TraceLevel::DEBUG, "handling voltage request...\n");
		float voltage = (*req->get_data()) & 3;
	
		int addr = req->get_addr();

		switch (addr)
		{
		// GENERATED VOLTAGE OFFSETS
		case 0:
			_this->voltage_ctrl_itf_host.sync(voltage);
			_this->trace.msg(vp::TraceLevel::DEBUG, "switching voltage of host to %f\n");
			break;

		// END GENERATED VOLTAGE OFFSETS
		default:
			_this->trace.msg(vp::TraceLevel::DEBUG, "No component associated with offset %d\n", addr);
			break;
		}
	}
	return vp::IoReqStatus::IO_REQ_OK;
}

extern "C" Component *gv_new(ComponentConf &config)
{
	return new PowerManager(config);
}
