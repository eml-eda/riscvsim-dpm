#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <vp/itf/wire.hpp>
#include "my_class.hpp"

class MyComp2 : public vp::Component
{

public:
    MyComp2(vp::ComponentConf &config);

private:
    static void handle_notif(vp::Block *__this, bool value);

    vp::WireMaster<MyClass *> result_itf;
    vp::WireSlave<bool> notif_itf;
};

MyComp2::MyComp2(vp::ComponentConf &config)
    : vp::Component(config)
{
    this->new_master_port("result", &this->result_itf);
    this->new_slave_port("notif", &this->notif_itf);
    this->notif_itf.set_sync_meth(&MyComp2::handle_notif);
}

void MyComp2::handle_notif(vp::Block *__this, bool value)
{
    MyComp2 *_this = (MyComp2 *)__this;
    printf("COMP2: Received value %d\n", value);

        MyClass result = {.value0 = 0x11111111, .value1 = 0x22222222};
    _this->result_itf.sync(&result);
}


extern "C" vp::Component *gv_new(vp::ComponentConf &config)
{
    return new MyComp2(config);
}