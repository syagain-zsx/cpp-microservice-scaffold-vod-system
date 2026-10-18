
#include <message.pb.h>
#include <filesystem>
#include "svc_mq.h"

namespace svc_file {
    // 实例化类成员，在实例化_subscriber的时候需要先构造对象，声明交换机和队列，订阅队列消息
    FileRemoveMQ::FileRemoveMQ(const SvcData::ptr &svc_data, 
        const bitemq::MQClient::ptr &mq_client,
        const bitemq::declare_settings &settings)
        : _svc_data(svc_data)
        , _subscriber(bitemq::MQFactory::create<bitemq::Subscriber>(mq_client, settings)) {
        _subscriber->consume(std::bind(&FileRemoveMQ::callback, this, std::placeholders::_1, std::placeholders::_2));
    }

    //订阅队列消息的回调处理函数：接收消息获取文件ID，删除文件数据和文件元信息
    void FileRemoveMQ::callback(const char *body, size_t len) {
        try {
            api_message::DeleteFileMsg msg;
            bool ret = msg.ParseFromArray(body, len);
            if (ret == false) {
                ERR("文件删除消息反序列化失败！");
                return;
            }
            // 遍历文件ID
            // 1. 获取文件元信息
            // 2. 根据文件元信息中的文件存储路径，通过fdfs_client删除文件
            // 3. 删除文件元信息
            int sz = msg.file_id_size();
            for (int i = 0; i < sz; ++i) {
                DBG("收到文件删除消息: {}", msg.file_id(i));
                auto file = _svc_data->getFileMeta(msg.file_id(i));
                if (file.get() == nullptr) {
                    ERR("文件ID[{}]不存在！", msg.file_id(i));
                    continue;
                }
                std::string path = file->path(); //获取存储路径进行删除
                remove_subfile(path); //删除子文件
                bitefdfs::FDFSClient::remove(path);
                _svc_data->removeFileMeta(msg.file_id(i)); 
                DBG("文件删除完毕: {}- {}", msg.file_id(i), path);
            }
        } catch(const std::exception &e) {
            ERR("文件删除消息处理异常: {}", e.what());
        }
    }

    void FileRemoveMQ::remove_subfile(const std::string &path) {
        //1. 对path文件路径的后缀名进行判断，是否是 ".m3u8"， 如果不是则返回
        std::string ext = std::filesystem::path(path).extension();
        if (ext != ".m3u8") { return; }
        DBG("要删除的文件是一个M3U8文件, 需要优先删除子文件: {}", path);
        //2. 将文件下载到本地，通过M3U8Info解析文件，获取子文件ID
        std::string tmp_path = "./" + biteutil::Random::code();
        bool ret = bitefdfs::FDFSClient::download_to_file(path, tmp_path);
        if (ret == false) {
            ERR("从FDFS下载文件失败: {}", path);
            return;
        }
        bitehls::M3U8Info m3u8(tmp_path);
        m3u8.parse();
        auto ts_pieces = m3u8.pieces();
        //3. 遍历子文件ID，删除子文件以及子文件的元信息
        for (auto &piece : ts_pieces) {
            // 1. 解析获取文件ID
            std::string url = piece.second;
            std::string file_id = url.substr(url.find_last_of("=") + 1);
            DBG("删除子文件: {}", file_id);
            // 2. 通过文件ID，获取文件元信息
            auto file = _svc_data->getFileMeta(file_id);
            if (!file) {
                ERR("文件ID[{}]不存在！", file_id);
                continue;
            }
            std::string sub_path = file->path();
            // 3. 通过元信息中的文件path，删除FDFS上的文件
            bitefdfs::FDFSClient::remove(sub_path);
            // 4. 删除文件元信息
            _svc_data->removeFileMeta(file_id);
        }
        // 5. 删除本地临时文件
        std::filesystem::remove(tmp_path);
    }
}