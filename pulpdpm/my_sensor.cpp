#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>
#include <stdio.h>

using namespace vp;

class MySensor : public Component
{
private:
    IoSlave input_itf;
    IoSlave power_req_itf;
    vp::IoReq *pending_req;
    vp::ClockEvent event;
    vp::PowerSource access_power;
    vp::PowerSource background_power;
    vp::Trace trace;
     vp::Signal<uint32_t> vcd_value;

public:
    MySensor(ComponentConf &config);
    void power_supply_set(vp::PowerSupplyState state);
    static IoReqStatus handle_req(Block *__this, IoReq *req);
    static void handle_event(vp::Block *__this, vp::ClockEvent *event);
};

MySensor::MySensor(ComponentConf &config) : Component(config), event(this, MySensor::handle_event), vcd_value(*this, "status", 32)
{
    this->input_itf.set_req_meth(&MySensor::handle_req);
    this->new_slave_port("input", &this->input_itf);
    
    this->traces.new_trace("trace", &this->trace);


    this->power.new_power_source("leakage", &background_power, this->get_js_config()->get("**/background_power"));
    this->power.new_power_source("access", &access_power, this->get_js_config()->get("**/access_power"));
    
    this->background_power.leakage_power_start();
}

IoReqStatus MySensor::handle_req(Block *__this, IoReq *req)
{
    MySensor *_this = (MySensor *)__this;
    _this->access_power.account_energy_quantum();
    if (!req->get_is_write() && req->get_addr() == 0 && req->get_size() == 4)
    {
        *(uint32_t *)req->get_data() = rand();
        req->inc_latency(2000);
        return vp::IO_REQ_OK;
    }
    return IO_REQ_OK;
}

void MySensor::handle_event(vp::Block *__this, vp::ClockEvent *event)
{
    MySensor *_this = (MySensor *)__this;

    *(uint32_t *)_this->pending_req->get_data() = rand();
    _this->pending_req->get_resp_port()->resp(_this->pending_req);
}

extern "C" Component *gv_new(ComponentConf &config)
{
    return new MySensor(config);
}

void MySensor::power_supply_set(vp::PowerSupplyState state)
{
    if (state == vp::PowerSupplyState::ON)
    {
        this->background_power.dynamic_power_start();
    }
    else
    {
        this->background_power.dynamic_power_stop();
    }
}