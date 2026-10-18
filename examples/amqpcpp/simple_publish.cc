/*
    基于AMQP-CPP库，实现消息的发送
        1. 实例化libev网络通信模块句柄
        2. 实例化Connection对象
        3. 实例化Channel对象
        4. 声明交换机
        5. 声明队列
        6. 绑定交换机和队列（采用直接交换）
        7. 发送消息到交换机
        8. 启动事件循环
*/

#include <amqpcpp.h>
#include <ev.h>
#include <amqpcpp/libev.h>
#include <iostream>

int main()
{
    // 0. 定义rabbitmq容器的访问url
    const std::string url = "amqp://admin:123456@192.168.65.128:5672/";
    const std::string exchange = "my-exchange";
    const std::string queue = "my-queue";
    const std::string binding_key = "my-binding-key";
    // 1. 实例化libev网络通信模块句柄
    auto *ev_loop = EV_DEFAULT; //网络通信事件循环句柄
    AMQP::LibEvHandler handler(ev_loop); //与amqpcpp库结合的句柄
    // 2. 实例化Connection对象
    AMQP::TcpConnection connection(&handler, AMQP::Address(url)); //与rabbitmq服务器的连接对象
    // 3. 实例化Channel对象
    AMQP::TcpChannel channel(&connection); //与rabbitmq服务器的通信通道对象
    // 4. 声明交换机:声明一个交换机，direct类型（直接交换
    channel.declareExchange(exchange, AMQP::ExchangeType::direct)
        .onSuccess([&](){
            std::cout << "声明交换机成功:" << exchange << std::endl;
            // 5. 声明队列
            channel.declareQueue(queue)
                .onSuccess([&](const std::string &name,
                     uint32_t messagecount, 
                     uint32_t consumercount){
                    std::cout << "声明队列成功:" << queue << std::endl;
                    std::cout << "队列中有" << messagecount << "个消息" << std::endl;
                    std::cout << "队列中有" << consumercount << "个消费者" << std::endl;

                    // 6. 绑定交换机和队列（采用直接交换）
                    channel.bindQueue(exchange, queue, binding_key)
                       .onSuccess([&](){
                            std::cout << "绑定交换机和队列成功:" << std::endl;
                            // 7. 发送消息到交换机
                            bool ret = channel.publish(exchange, binding_key, "Hello World");
                            if(ret == false) {
                                std::cout << "发送消息失败" << std::endl;
                            }
                       })
                       .onError([&](const char *message) {
                           std::cout << "绑定交换机和队列失败:" << message << std::endl;
                           abort(); // 终止程序
                       });
                })
                .onError([&](const char *message) {
                    std::cout << "声明队列失败:" << message << std::endl;
                    abort(); // 终止程序
                });
        })
        .onError([&](const char *message) {
            std::cout << "声明交换机失败:" << message << std::endl;
            abort(); // 终止程序
        });
    
    // 8. 启动事件循环
    ev_run(ev_loop);
    return 0;
}
