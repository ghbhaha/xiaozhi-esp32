#ifndef _SYSTEM_INFO_H_
#define _SYSTEM_INFO_H_

#include <string>

#include <esp_err.h>
#include <freertos/FreeRTOS.h>



// 定义固定的MAC地址数组
#define MAC_ADDRESSES { \
    "7c:86:45:86:c1:ab", \
    "88:f6:b3:77:43:3e", \
    "02:bc:52:3c:87:39", \
    "13:f0:80:dc:2f:25"  \
}

//定义UUID数组
#define UUID_ADDRESSES { \
    "6ba7b810-9dad-11d1-80b4-00c04fd430c8", \
    "7c9e6679-7425-40de-944b-e07fc1f90ae7", \
    "8b91a6d9-1380-4af2-b908-34c4153df553", \
    "9ed66e44-7925-4c10-94c3-d5a887149d72"  \
}

//定义智能体名字数组
#define AGENT_NAMES { \
    "霸总", \
    "宝儿姐", \
    "英语陪练", \
    "儿童故事机"  \
}


class SystemInfo {
public:
    //mac index减少
    static void DecreaseMacIndex();
    //mac index增加
    static void IncreaseMacIndex();

    static size_t GetFlashSize();
    static size_t GetMinimumFreeHeapSize();
    static size_t GetFreeHeapSize();
    static std::string GetMacAddress();
    static std::string GetUuid();
    static std::string GetAgentName();
    static std::string GetChipModelName();
    static esp_err_t PrintTaskCpuUsage(TickType_t xTicksToWait);
    static void PrintTaskList();
    static void PrintHeapStats();
};

#endif // _SYSTEM_INFO_H_
