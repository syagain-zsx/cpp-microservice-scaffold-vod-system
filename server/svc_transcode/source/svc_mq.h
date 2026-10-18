#pragma once
#include "svc_data.h"
#include "svc_worker.h"
#include <JianLiuScaffold/mq.h>
#include <JianLiuScaffold/log.h>
#include <JianLiuScaffold/hls.h>
#include <JianLiuScaffold/fdfs.h>

namespace svc_transcode {

    class TranscodeMQ {
        public:
            using ptr = std::shared_ptr<TranscodeMQ>;
            TranscodeMQ(const SvcData::ptr &svc_data,
                const JianLiuHls::HLSTranscoder::ptr &transcoder,
                const JianLiuMq::MQClient::ptr &mq_client,
                const JianLiuMq::declare_settings &trans_settings,
                const std::string &temp_path,
                const std::string &access_url);
            bool transcode(const std::string &video_id);
        private:
            void callback(const char *body, size_t len);
        private:
            std::string _access_url;
            std::string _temp_path;// 临时文件存放路径（存放下载下来的视频文件，以及转码处理时生成的文件）
            SvcData::ptr _svc_data;
            SvcWorker::ptr _worker;
            // JianLiuFdfs::FDFSClient::ptr _fdfs_client;  这是一个方法类，不需要实例化对象
            JianLiuHls::HLSTranscoder::ptr _transcoder;
            JianLiuMq::Subscriber::ptr _subscriber;//消息订阅者对象
    };
}