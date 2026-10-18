#include <video_search.h>

int main()
{
    try {
        const std::string url = "http://elastic:123456@192.168.65.128:9200/"; 
        bitelog::bitelog_init();
        std::vector<std::string> urls = {url};
        auto client = std::make_shared<biteics::ESClient>(urls);
        vp_data::VideoSearchData vsd(client);
        // {
        //     //1. 新增视频信息
        //     vsd.insert("video_1", "可爱的猫猫", "这是一只可爱的猫猫喵喵叫", true);
        //     vsd.insert("video_2", "可爱的狗狗", "这是一只可爱的狗狗汪汪叫", true);
        //     vsd.insert("video_3", "凶猛的狼", "凶猛的狼也会汪汪叫", true);
        //     vsd.insert("video_4", "凶猛的老虎", "凶猛的老虎也会喵喵叫", true);
        //     vsd.insert("video_5", "迅捷的猎豹", "猎豹也会喵喵叫", true);
        //     vsd.insert("video_6", "胖胖的小猪", "小猪只会哼哼叫", true);
        // }
        // {
        //     //2. 更新视频信息
        //     vsd.update("video_5", "迅捷的猎豹", "猎豹也会喵喵叫", true);
        // }
        // {
        //     //3. 搜索视频信息
        //     auto res = vsd.search("喵喵", 0, 10);
        //     for (auto &i : res) {
        //         std::cout << i << std::endl;
        //     }
        // }
        // {
        //     //4. 删除视频信息
        //     vsd.remove("video_6");
        // }
        {
            //5. 删除索引
            vsd.remove();
        }
    }catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}