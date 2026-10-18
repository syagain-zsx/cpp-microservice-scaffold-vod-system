#pragma once 
#include <video.h>
#include <file.h>

namespace svc_transcode {
    class SvcData {
        public:
            using ptr = std::shared_ptr<SvcData>;
            SvcData(const std::shared_ptr<odb::database> &mysql);
            vp_data::Video::ptr getVideo(const std::string &vid);
            vp_data::File::ptr getFile(const std::string &fid);
            void setVideo(vp_data::Video::ptr &video);
            void setFile(vp_data::File::ptr &file);
            void newFile(vp_data::File &file);
        private:
            std::shared_ptr<odb::database> _mysql;
    };
}