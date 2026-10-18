#pragma once
#include <bite_scaffold/odb.h>
#include <bite_scaffold/redis.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <bite_scaffold/util.h>
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>


namespace vp_data{
    class BarrageData {
        public:
            using ptr = std::shared_ptr<BarrageData>;
            BarrageData(odb::transaction &mtx);
            void insert(Barrage &barrage);
            void remove(const std::string &video_id);
            std::vector<Barrage::ptr> list(const std::string &video_id);
        private:
            odb::database& _mysql;
    };
}