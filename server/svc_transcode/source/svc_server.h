#include <bite_scaffold/log.h>
#include "svc_mq.h"
#include "svc_data.h"


namespace svc_transcode {
    class TranscodeServer {
        public:
            using ptr = std::shared_ptr<TranscodeServer>;
            TranscodeServer(const TranscodeMQ::ptr &tmq,
                const bitemq::MQClient::ptr &mqclient);
            void wait();
        private:
            TranscodeMQ::ptr _tmq;
            bitemq::MQClient::ptr _mq_client;
    };

    class TranscodeServerBuilder {
        public:
            TranscodeServerBuilder() = default;
            ~TranscodeServerBuilder() = default;
            TranscodeServerBuilder &withTempDir(const std::string &path);
            TranscodeServerBuilder &withHLSBaseUrl(const std::string &base_url);
            TranscodeServerBuilder &withMqUrl(const std::string &mq_url);
            TranscodeServerBuilder &withMqTranscodeSettings(const std::string &mq_settings);
            TranscodeServerBuilder &withMysqlSettings(const biteodb::mysql_settings &mysql_settings);
            TranscodeServerBuilder &withFDFSSettings(const bitefdfs::fdfs_settings &fdfs_settings);
            TranscodeServerBuilder &withHLSSettings(const bitehls::hls_setttings &hls_settings);
            TranscodeServer::ptr build();
        private:
            std::string _temp_dir;
            std::string _base_url;
            std::string _mq_url;
            bitemq::declare_settings _mq_transcode_settings;
            biteodb::mysql_settings _mysql_settings;
            bitefdfs::fdfs_settings _fdfs_settings;
            bitehls::hls_setttings _hls_settings;
    };
}