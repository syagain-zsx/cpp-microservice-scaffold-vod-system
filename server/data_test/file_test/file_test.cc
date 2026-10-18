#include <file.h>

int main()
{
    //1. 实例化database对象
    biteodb::mysql_settings settings = {
        .host = "192.168.65.131",
        .passwd = "123456",
        .db = "vptest",
        .connection_pool_size = 5
    };
    auto handler = biteodb::DBFactory::mysql(settings);
    {
        try {
            odb::transaction tx(handler->begin());
            auto &db = tx.database();
            //2. 实例化FileData对象
            vp_data::FileData file_handler(db);
            //3. 测试文件数据的增删改查
            // vp_data::File file("file_id-1", "uploader_uid-1", "path-1", 2048, "mime-1");
            // file_handler.insert(file);
            // std::cout << "数据新增成功！" << std::endl;
            // auto file1 = file_handler.selectById("file_id-1");
            // if (file1) {
            //     std::cout << "数据查询成功！" << std::endl;
            //     std::cout << file1->file_id() << std::endl;
            //     std::cout << file1->uploader_uid() << std::endl;
            //     std::cout << file1->path() << std::endl;
            //     std::cout << file1->size() << std::endl;
            //     std::cout << file1->mime() << std::endl;
            // }else {
            //     std::cout << "数据查询失败！" << std::endl;
            // }
            // file1->set_uploader_uid("uploader_uid-2");
            // file_handler.update(*file1);
            file_handler.remove("file_id-1");
            tx.commit();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}