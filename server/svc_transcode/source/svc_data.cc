#include "svc_data.h"

namespace svc_transcode {
    SvcData::SvcData(const std::shared_ptr<odb::database> &mysql): _mysql(mysql) {}
    vp_data::Video::ptr SvcData::getVideo(const std::string &vid) {
        vp_data::Video::ptr video;
        try {
            odb::transaction mtx(_mysql->begin());
            vp_data::VideoDBData vdd(mtx);
            video = vdd.get(vid);
            mtx.commit();
        } catch (const vp_error::VPException &e) {
            throw;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
        return video;
    }
    vp_data::File::ptr SvcData::getFile(const std::string &fid) {
        vp_data::File::ptr file;
        try {
            odb::transaction mtx(_mysql->begin());
            vp_data::FileData fd(mtx);
            file = fd.selectById(fid);
            mtx.commit();
            if (!file) {
                throw vp_error::VPException(vp_error::VpError::FILE_NOT_EXIST);
            }
        } catch (const vp_error::VPException &e) {
            throw;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
        return file;
    }
    void SvcData::setVideo(vp_data::Video::ptr &video) {
        try {
            odb::transaction mtx(_mysql->begin());
            vp_data::VideoDBData vdd(mtx);
            vdd.set(video);
            mtx.commit();
        } catch (const vp_error::VPException &e) {
            throw;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
    }
    void SvcData::setFile(vp_data::File::ptr &file) {
        try {
            odb::transaction mtx(_mysql->begin());
            vp_data::FileData fd(mtx);
            fd.update(*file);
            mtx.commit();
        } catch (const vp_error::VPException &e) {
            throw;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
    }
    void SvcData::newFile(vp_data::File &file) {
        try {
            odb::transaction mtx(_mysql->begin());
            vp_data::FileData fd(mtx);
            fd.insert(file);
            mtx.commit();
        } catch (const vp_error::VPException &e) {
            throw;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
    }
}