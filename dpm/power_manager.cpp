#include <vp/vp.hpp>
#include <vp/signal.hpp>
#include <vp/itf/io.hpp>

#define time_sensor_1 50
#define time_sensor_2 150
#define time_sensor_3 520
#define time_comp 20

using namespace vp;

typedef enum state
{
    read1,
    read2,
    read3,
    comp
} state;

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
    vp::ClockEvent event;
    WireMaster<int> power_ctrl_itf;
    WireMaster<int> voltage_ctrl_itf;
    WireMaster<int> power_ctrl_itf1;
    WireMaster<int> voltage_ctrl_itf1;
    WireMaster<int> power_ctrl_itf2;
    WireMaster<int> voltage_ctrl_itf2;
    WireMaster<int> power_ctrl_itf3;
    WireMaster<int> voltage_ctrl_itf3;
};

PowerManager::PowerManager(ComponentConf &config)
    : Component(config), event(this, PowerManager::handle_event)
{

    this->new_slave_port("input", &this->input_itf);

    this->new_master_port("power_ctrl", &this->power_ctrl_itf);

    this->new_master_port("voltage_ctrl", &this->voltage_ctrl_itf);

    this->new_master_port("power_ctrl1", &this->power_ctrl_itf1);

    this->new_master_port("voltage_ctrl1", &this->voltage_ctrl_itf1);

    this->new_master_port("power_ctrl2", &this->power_ctrl_itf2);

    this->new_master_port("voltage_ctrl2", &this->voltage_ctrl_itf2);

    this->new_master_port("power_ctrl3", &this->power_ctrl_itf3);

    this->new_master_port("voltage_ctrl3", &this->voltage_ctrl_itf3);

    this->traces.new_trace("trace", &this->trace);
}

// IoReqStatus PowerManager::handle_req(Block *__this, IoReq *req)
// {
//     PowerManager *_this = (PowerManager *)__this;

//     /*TODO receive request and set power state*/
//     return IO_REQ_OK;
// }

void PowerManager::reset(bool active)
{
    if (active)
    {
        this->power_ctrl_itf.sync(PowerSupplyState::OFF);
    }
    else
        this->event.enqueue(1);
}

void PowerManager::start(){
    this->event.enqueue(10);
}

// void PowerManager::handle_notif(vp::Block *__this, bool value)
// {
//     PowerManager *_this = (PowerManager *)__this;

//     _this->trace.msg(vp::TraceLevel::DEBUG, "Received notif\n");

//     if (!_this->event.is_enqueued())
//     {
//         _this->event.enqueue(1);
//     }
// }

void PowerManager::handle_event(vp::Block *__this, vp::ClockEvent *event)
{
    static state curstate = read1;
    PowerManager *_this = (PowerManager *)__this;

    switch (curstate)
    {
    case read1:
        if (!_this->event.is_enqueued())
        {
            _this->trace.msg(vp::TraceLevel::DEBUG, "inside read1\n");
            _this->power_ctrl_itf.sync(vp::PowerSupplyState::OFF);
            _this->power_ctrl_itf1.sync(vp::PowerSupplyState::ON);
            _this->event.enqueue(time_sensor_1);
            curstate = read2;
        }
        break;
    case read2:
        if (!_this->event.is_enqueued())
        {
                        _this->trace.msg(vp::TraceLevel::DEBUG, "inside read2\n");
            _this->power_ctrl_itf1.sync(vp::PowerSupplyState::OFF);
            _this->power_ctrl_itf2.sync(vp::PowerSupplyState::ON);
            _this->event.enqueue(time_sensor_2);
            curstate = read3;
        }
        break;
    case read3:
        if (!_this->event.is_enqueued())
        {
                        _this->trace.msg(vp::TraceLevel::DEBUG, "inside read3\n");
            _this->power_ctrl_itf2.sync(vp::PowerSupplyState::OFF);
            _this->power_ctrl_itf3.sync(vp::PowerSupplyState::ON);
            _this->event.enqueue(time_sensor_3);
            curstate = comp;
        }
        break;
    case comp:
        if (!_this->event.is_enqueued())
        {
                        _this->trace.msg(vp::TraceLevel::DEBUG, "inside read4\n");
            _this->power_ctrl_itf3.sync(vp::PowerSupplyState::OFF);
            _this->power_ctrl_itf.sync(vp::PowerSupplyState::ON);
            _this->event.enqueue(time_comp);
            curstate = read1;
        }
        break;
    default:
        curstate = read1;
        break;
    }
}

extern "C" Component *gv_new(ComponentConf &config)
{
    return new PowerManager(config);
}
