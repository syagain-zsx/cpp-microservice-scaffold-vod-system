#pragma once
#include "data.h"
#include <data-odb.hxx>
#include <bite_scaffold/odb.h>

namespace vp_data{
    class FileData {
        public:
            using ptr = std::shared_ptr<FileData>;
            FileData(odb::transaction &mtx);
            void insert(File &file);
            // 更新操作传入的对象，必须是查询操作获取到的对象
            void update(File &file);
            void remove(const std::string &file_id);
            File::ptr selectById(const std::string &file_id);
        private:
            odb::database &_db;
    };
}
