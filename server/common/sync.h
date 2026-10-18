#pragma once
#include <iostream>
#include <string>

namespace vp_sync {
    class CacheSync {
        public:
            using ptr = std::shared_ptr<CacheSync>;
            virtual void sync(const std::string &key) = 0;
    };
}