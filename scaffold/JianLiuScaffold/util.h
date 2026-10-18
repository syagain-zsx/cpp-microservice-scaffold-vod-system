/*
    util工具封装
        1. json序列化和反序列化
*/

#pragma once
#include <jsoncpp/json/json.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <optional>
#include "log.h"

namespace JianLiuUtil {
    class JSON {  
        public:
            static std::optional<std::string> serialize(const Json::Value& val, bool styled = false);
            static std::optional<Json::Value> unserialize(const std::string& input);
    };

    class FUTIL { 
        public:
            static bool read(const std::string &filename, std::string &body);
            static bool write(const std::string &filename, const std::string &body);
    };

    class STR {
        public:
            static size_t split(const std::string &src, const std::string &sep, std::vector<std::string> &dst);

    };
    
    const size_t UUID_SIZE = 16;
    enum UuidType {
        MIX = 0,
        CHAR,
        DIGIT
    };
    class Random {
        public:
            //16个字符长度的字母+数字随机字符串
            static std::string code(size_t len = UUID_SIZE, UuidType utype = UuidType::MIX);
            //生成一个指定区间的随机数
            static size_t number(size_t min, size_t max);
    };

    // 密码哈希：加盐 SHA-256（截断 128 位），输出 64 位十六进制（32 位盐 + 32 位摘要）
    class Password {
        public:
            static std::string hash(const std::string &plain);
            // 校验；兼容历史明文密码（stored 不是 64 位十六进制时按明文比较）
            static bool verify(const std::string &stored, const std::string &plain);
    };
}