#include <httplib.h>

int main()
{
    httplib::Client client("192.168.65.128", 9000);
    httplib::Headers headers = {
        {"Connection", "closed"}
    };
    httplib::Params params = {
        {"name", "lisi"},
        {"age", "18"}
    };
    auto rsp = client.Get("/hi", params, headers);
    if (rsp && rsp->status == 200) {
        std::cout << rsp->body << std::endl;
    }
    return 0;
}