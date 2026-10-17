#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using namespace std;

typedef websocketpp::server<websocketpp::config::asio> websocketsvr;
typedef websocketsvr::message_ptr message_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// websocket连接成功的回调函数
void OnOpen(websocketsvr *server,websocketpp::connection_hdl hdl){
    cout<<"连接成功"<<endl;
}

// websocket连接成功的回调函数
void OnClose(websocketsvr *server,websocketpp::connection_hdl hdl){
    cout<<"连接关闭"<<endl;
}

// websocket连接收到消息的回调函数
void OnMessage(websocketsvr *server,websocketpp::connection_hdl hdl,message_ptr msg){
    cout << "收到消息" << msg->get_payload() << endl;
    // 收到消息将相同的消息发回给websocket客户端
    server->send(hdl, msg->get_payload(), websocketpp::frame::opcode::text);
}

// websocket连接异常的回调函数
void OnFail(websocketsvr *server,websocketpp::connection_hdl hdl){
    cout<<"连接异常"<<endl;
}

// 处理http请求的回调函数 返回一个html欢迎页面
void OnHttp(websocketsvr *server,websocketpp::connection_hdl hdl){
    cout<<"处理http请求"<<endl;
    websocketsvr::connection_ptr con = server->get_con_from_hdl(hdl);
    std::stringstream ss;
    ss << "<!doctype html><html><head>"
        << "<title>hello websocket</title><body>"
        << "<h1>hello websocketpp</h1>"
        << "</body></head></html>";
    con->set_body(ss.str());
    con->set_status(websocketpp::http::status_code::ok);
}

int main(){
    // 使用websocketpp库创建服务器
    websocketsvr server;
    // 设置websocketpp库的日志级别
    // all表示打印全部级别日志
    // none表示什么日志都不打印
    server.set_access_channels(websocketpp::log::alevel::none);
    /*初始化asio*/
    server.init_asio();
    // 注册http请求的处理函数
    server.set_http_handler(bind(&OnHttp, &server, ::_1));
    // 注册websocket请求的处理函数
    server.set_open_handler(bind(&OnOpen, &server, ::_1));
    server.set_close_handler(bind(&OnClose, &server, _1));
    server.set_message_handler(bind(&OnMessage,&server,_1,_2));
    // 监听8888端口
    server.listen(8888);
    // 开始接收tcp连接
    server.start_accept();
    // 开始运行服务器
    server.run();
    return 0;
}
