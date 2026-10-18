#include <JianLiuScaffold/log.h>
#include "svc_mq.h"
#include "svc_data.h"


namespace svc_transcode {
    class TranscodeServer {
        public:
            using ptr = std::shared_ptr<TranscodeServer>;
            TranscodeServer(const TranscodeMQ::ptr &tmq,
                const JianLiuMq::MQClient::ptr &mqclient);
            void wait();
        private:
            TranscodeMQ::ptr _tmq;
            JianLiuMq::MQClient::ptr _mq_client;
    };

    class TranscodeServerBuilder {
        public:
            TranscodeServerBuilder() = default;
            ~TranscodeServerBuilder() = default;
            TranscodeServerBuilder &withTempDir(const std::string &path);
            TranscodeServerBuilder &withHLSBaseUrl(const std::string &base_url);
            TranscodeServerBuilder &withMqUrl(const std::string &mq_url);
            TranscodeServerBuilder &withMqTranscodeSettings(const std::string &mq_settings);
            TranscodeServerBuilder &withMysqlSettings(const JianLiuOdb::mysql_settings &mysql_settings);
            TranscodeServerBuilder &withFDFSSettings(const JianLiuFdfs::fdfs_settings &fdfs_settings);
            TranscodeServerBuilder &withHLSSettings(const JianLiuHls::hls_setttings &hls_settings);
            TranscodeServer::ptr build();
        private:
            std::string _temp_dir;
            std::string _base_url;
            std::string _mq_url;
            JianLiuMq::declare_settings _mq_transcode_settings;
            JianLiuOdb::mysql_settings _mysql_settings;
            JianLiuFdfs::fdfs_settings _fdfs_settings;
            JianLiuHls::hls_setttings _hls_settings;
    };
}