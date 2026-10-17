/*
    使用HTTPLIB搭建HTTP服务器：
      1. 包含头文件
      2. 实例化Server对象
      3. 注册路由信息 --- 告诉httplib收到哪个请求应该由哪个接口处理
      4. 启动服务器
*/


#include <httplib.h>

void HelloWorld(const httplib::Request &req, httplib::Response &rsp) {
    std::cout << req.method << std::endl;
    std::cout << req.path << std::endl;
    std::cout << req.body << std::endl;
    for (auto it : req.headers) {
        std::cout << it.first << " = " << it.second << std::endl;
    }
    for (auto it : req.params) {
        std::cout << it.first << " = " << it.second << std::endl;
    }
    std::string html_body = "<html><body><h1>Hello World</h1></body></html>";
    rsp.set_content(html_body, "text/html");
    rsp.status = 200; 
    return;
}

int main()
{
    httplib::Server svr;
    svr.Get("/hi", HelloWorld);
    svr.Get(R"(/numbers/(\d+))", [](const httplib::Request &req, httplib::Response &rsp){
        std::cout << req.method << std::endl;
        std::cout << req.path << std::endl;
        for (auto it : req.matches) {
            std::cout << it << std::endl;
        }
    });
    svr.listen("0.0.0.0", 9000);
    return 0;
}