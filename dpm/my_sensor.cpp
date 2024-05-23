#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <stdio.h>

using namespace vp;

class MySensor : public Component
{
private:
    IoSlave input_itf;
    static IoReqStatus handle_req(Block *__this, IoReq *req);
public:
    MySensor(ComponentConf &config);


};

MySensor::MySensor(ComponentConf &config) : Component(config)
{
    this->input_itf.set_req_meth(&MySensor::handle_req);
    this->new_slave_port("input", &this->input_itf);
}



extern "C" Component *gv_new(ComponentConf &config){
    return new MySensor(config);
}

IoReqStatus MySensor::handle_req(Block *__this, IoReq *req)
{
    MySensor *_this = (MySensor *)__this;

    if (!req->get_is_write() && req->get_addr() == 0 && req->get_size() == 4)
    {

        *(uint32_t *)req->get_data() = rand();
    }
    return IO_REQ_OK;
}


