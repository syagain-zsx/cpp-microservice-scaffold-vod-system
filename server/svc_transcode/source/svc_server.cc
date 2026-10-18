#include "svc_server.h"


namespace svc_transcode {
    TranscodeServer::TranscodeServer(const TranscodeMQ::ptr &tmq,
        const bitemq::MQClient::ptr &mqclient)
        : _tmq(tmq)
        , _mq_client(mqclient){}
    void TranscodeServer::wait() { _mq_client->wait(); }


    TranscodeServerBuilder &TranscodeServerBuilder::withTempDir(const std::string &path) {
        _temp_dir = path;
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withHLSBaseUrl(const std::string &base_url) {
        _base_url = base_url;
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withMqUrl(const std::string &mq_url) {
        _mq_url = mq_url;
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withMqTranscodeSettings(const std::string &mq_settings) {
        auto queue_info = biteutil::JSON::unserialize(mq_settings);
        if (!queue_info) {
            ERR("queue_info 格式错误");
            abort();
        }
        _mq_transcode_settings.exchange = (*queue_info)["exchange"].asString();
        _mq_transcode_settings.exchange_type = (*queue_info)["type"].asString();
        _mq_transcode_settings.queue = (*queue_info)["queue"].asString();
        _mq_transcode_settings.binding_key = (*queue_info)["binding_key"].asString();
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withMysqlSettings(const biteodb::mysql_settings &mysql_settings) {
        _mysql_settings = mysql_settings;
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withFDFSSettings(const bitefdfs::fdfs_settings &fdfs_settings) {
        _fdfs_settings = fdfs_settings;
        return *this;
    }
    TranscodeServerBuilder &TranscodeServerBuilder::withHLSSettings(const bitehls::hls_setttings &hls_settings) {
        _hls_settings = hls_settings;
        return *this;
    }
    TranscodeServer::ptr TranscodeServerBuilder::build() {
        auto mysql = biteodb::DBFactory::mysql(_mysql_settings);
        auto svc_data = std::make_shared<svc_transcode::SvcData>(mysql);
        auto transcoder = std::make_shared<bitehls::HLSTranscoder>(_hls_settings);
        bitefdfs::FDFSClient::init(_fdfs_settings);
        auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(_mq_url);
        auto transcode_mq = std::make_shared<TranscodeMQ>(svc_data, transcoder, mq_client, _mq_transcode_settings, _temp_dir, _base_url);

        auto server = std::make_shared<TranscodeServer>(transcode_mq, mq_client);
        return server;
    }
}