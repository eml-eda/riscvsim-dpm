#include <cstring>
#include <gv/gvsoc.hpp>
#include <vp/launcher.hpp>


class MyLauncher : public gv::Io_user
{
public:
    int run(std::string config_path);

    // This gets called when an access from gvsoc side is reaching us
    void access(gv::Io_request *req);
    // This gets called when one of our access gets granted
    void grant(gv::Io_request *req);
    // This gets called when one of our access gets its response
    void reply(gv::Io_request *req);

private:
    gv::Io_binding *axi;
    gv::Io_binding *axi_pm;
};



int main(int argc, char *argv[])
{
    char *config_path = NULL;

    for (int i=1; i<argc; i++)
    {
        if (strncmp(argv[i], "--config=", 9) == 0)
        {
            config_path = &argv[i][9];
        }
    }

    if (config_path == NULL)
    {
        fprintf(stderr, "No configuration specified, please specify through option --config=<config path>.\n");
        return -1;
    }

    MyLauncher launcher;

    return launcher.run(config_path);
}



int MyLauncher::run(std::string config_path)
{

    gv::GvsocConf conf = { .config_path=config_path, .api_mode=gv::Api_mode::Api_mode_sync };
   
    gv::GvsocLauncher *gvsoc = (gv::GvsocLauncher *)gv::gvsoc_new(&conf);
    gvsoc->open();
    // Get a connection to the main soc AXI. This will allow us to inject accesses
    // and could also be used to received accesses from simulated test
    // to a certain mapping corresponding to the external devices.
    this->axi = gvsoc->io_bind(this, "/host/chip/axi_proxy", "");
    if (this->axi == NULL)
    {
        fprintf(stderr, "Couldn't find AXI proxy\n");
        return -1;
    }

      this->axi_pm = gvsoc->io_bind(this, "/axi_pm", "");
    if (this->axi_pm == NULL)
    {
        fprintf(stderr, "Couldn't find AXI pm proxy\n");
        return -1;
    }

    gvsoc->start();

    // Run
    gvsoc->run();
    

    // Wait for simulation termination and exit code returned by simulated test
    int retval = gvsoc->join();

    gvsoc->stop();
    gvsoc->close();

    return retval;
}



void MyLauncher::access(gv::Io_request *req)
{
    printf("Received request (is_read: %d, addr: 0x%lx, size: 0x%lx)\n", req->type == gv::Io_request_read, req->addr, req->size);

    if (req->type == gv::Io_request_read)
    {
        memset(req->data, req->addr, req->size);
    }

    this->axi->reply(req);
    this->axi_pm->access(req);
}



void MyLauncher::grant(gv::Io_request *req)
{
}



void MyLauncher::reply(gv::Io_request *req)
{
}

