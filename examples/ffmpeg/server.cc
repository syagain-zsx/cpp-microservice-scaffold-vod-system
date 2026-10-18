#include <httplib.h>


int main()
{
    httplib::Server svr;
    svr.set_mount_point("/", "./wwwroot/");
    svr.listen("0.0.0.0", 9000);
    return 0;
}