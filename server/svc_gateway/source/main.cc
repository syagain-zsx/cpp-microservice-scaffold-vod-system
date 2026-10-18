#include <bite_scaffold/util.h>
#include <bite_scaffold/log.h>
#include "svc_server.h"

DEFINE_int32(log_level, 1, "日志等级: 1-debug;2-info;3-warn;4-error; 6-off");
DEFINE_bool(log_async, false, "日志是否异步"); 
DEFINE_string(log_format, "[%H:%M:%S][%-7l]: %v", "日志格式");
DEFINE_string(log_path, "stdout", "日志输出目标");

DEFINE_int32(listen_port, 9000, "文件子服务监听端口");

DEFINE_string(registry_center_addr, "http://192.168.65.128:2379", "注册中心地址");
DEFINE_string(user_svc_name, "user", "用户子服务名称");
DEFINE_string(file_svc_name, "file", "文件子服务名称");
DEFINE_string(video_svc_name, "video", "视频子服务名称");

DEFINE_int32(mysql_port, 3306, "mysql端口");
DEFINE_int32(mysql_connection_pool_size, 5, "mysql连接池大小");
DEFINE_string(mysql_host, "192.168.65.128", "mysql服务器地址");
DEFINE_string(mysql_user, "root", "mysql用户名");
DEFINE_string(mysql_passwd, "123456", "mysql密码");
DEFINE_string(mysql_db, "vptest", "mysql数据库名");

DEFINE_int32(redis_port, 6379, "redis端口");
DEFINE_int32(redis_connection_pool_size, 5, "redis连接池大小");
DEFINE_string(redis_host, "192.168.65.128", "redis服务器地址");
DEFINE_string(redis_passwd, "123456", "redis密码");
DEFINE_string(redis_user, "default", "redis用户名");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    bitelog::log_settings  lsettings = {
        .async = FLAGS_log_async,
        .level = FLAGS_log_level,
        .format = FLAGS_log_format,
        .path = FLAGS_log_path,
    };
    bitelog::bitelog_init(lsettings);
   
    biteodb::mysql_settings msettings = {
        .host =     FLAGS_mysql_host,
        .user =     FLAGS_mysql_user,
        .passwd =   FLAGS_mysql_passwd,
        .db =       FLAGS_mysql_db,
        .connection_pool_size = (unsigned int)FLAGS_mysql_connection_pool_size
    };
    biteredis::redis_settings rsettings = {
        .port = FLAGS_redis_port,
        .host = FLAGS_redis_host,
        .user = FLAGS_redis_user,
        .passwd = FLAGS_redis_passwd,
        .connection_pool_size = (size_t)FLAGS_redis_connection_pool_size
    };

    svc_gateway::discovery_settings dis_settings = {
        .registry_center_addr = FLAGS_registry_center_addr,
        .user_svc_name = FLAGS_user_svc_name,
        .file_svc_name = FLAGS_file_svc_name,
        .video_svc_name = FLAGS_video_svc_name
    };

    svc_gateway::GatewayServerBuilder builder;
    auto server = builder.withListenPort(FLAGS_listen_port)
        .withDiscoverySettings(dis_settings)
        .withMysqlSettings(msettings)
        .withRedisSettings(rsettings)
        .build();
    //启动服务
    server->start();
    return 0;
}