#include "system_info.h"

#include <freertos/task.h>
#include <esp_log.h>
#include <esp_flash.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <esp_partition.h>
#include <esp_app_desc.h>
#include <esp_ota_ops.h>
#include "settings.h"


#define TAG "SystemInfo"

size_t SystemInfo::GetFlashSize() {
    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get flash size");
        return 0;
    }
    return (size_t)flash_size;
}

size_t SystemInfo::GetMinimumFreeHeapSize() {
    return esp_get_minimum_free_heap_size();
}

size_t SystemInfo::GetFreeHeapSize() {
    return esp_get_free_heap_size();
}


void SystemInfo::DecreaseMacIndex() {
    Settings settings("macuuid", true);
    int mac_index = settings.GetInt("mac_index", 0);
    const char* mac_addresses[] = MAC_ADDRESSES;
    int mac_addresses_size = sizeof(mac_addresses) / sizeof(mac_addresses[0]);
    mac_index = mac_index % mac_addresses_size;
    if (mac_index == 0) {
        mac_index = mac_addresses_size - 1;
    } else {
        mac_index--;
    }
    settings.SetInt("mac_index", mac_index);
}

void SystemInfo::IncreaseMacIndex() {
    Settings settings("macuuid", true);
    int mac_index = settings.GetInt("mac_index", 0);
    const char* mac_addresses[] = MAC_ADDRESSES;
    int mac_addresses_size = sizeof(mac_addresses) / sizeof(mac_addresses[0]);
    mac_index = mac_index % mac_addresses_size;
    if (mac_index == mac_addresses_size - 1) {
        mac_index = 0;
    } else {
        mac_index++;
    }
    settings.SetInt("mac_index", mac_index);
}


std::string SystemInfo::GetMacAddress() {
    const char* mac_addresses[] = MAC_ADDRESSES;
    Settings settings("macuuid", true);
    int mac_addresses_size = sizeof(mac_addresses) / sizeof(mac_addresses[0]);
    int mac_index = settings.GetInt("mac_index", 0);
    mac_index = mac_index % mac_addresses_size;
    return std::string(mac_addresses[mac_index]);
}

std::string SystemInfo::GetUuid() {
    const char* uuid_addresses[] = UUID_ADDRESSES;
    Settings settings("macuuid", true);
    int uuid_addresses_size = sizeof(uuid_addresses) / sizeof(uuid_addresses[0]);
    int mac_index = settings.GetInt("mac_index", 0);
    mac_index = mac_index % uuid_addresses_size;
    return std::string(uuid_addresses[mac_index]);
}

std::string SystemInfo::GetAgentName() {
    const char* agent_names[] = AGENT_NAMES;
    Settings settings("macuuid", true);
    int agent_names_size = sizeof(agent_names) / sizeof(agent_names[0]);
    int mac_index = settings.GetInt("mac_index", 0);
    mac_index = mac_index % agent_names_size;
    return std::string(agent_names[mac_index]);
}

std::string SystemInfo::GetChipModelName() {
    return std::string(CONFIG_IDF_TARGET);
}

esp_err_t SystemInfo::PrintRealTimeStats(TickType_t xTicksToWait) {
    #define ARRAY_SIZE_OFFSET 5
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    configRUN_TIME_COUNTER_TYPE start_run_time, end_run_time;
    esp_err_t ret;
    uint32_t total_elapsed_time;

    //Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array = (TaskStatus_t*)malloc(sizeof(TaskStatus_t) * start_array_size);
    if (start_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    vTaskDelay(xTicksToWait);

    //Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array = (TaskStatus_t*)malloc(sizeof(TaskStatus_t) * end_array_size);
    if (end_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    //Calculate total_elapsed_time in units of run time stats clock period.
    total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        ret = ESP_ERR_INVALID_STATE;
        goto exit;
    }

    printf("| Task | Run Time | Percentage\n");
    //Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                //Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                break;
            }
        }
        //Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * CONFIG_FREERTOS_NUMBER_OF_CORES);
            printf("| %-16s | %8lu | %4lu%%\n", start_array[i].pcTaskName, task_elapsed_time, percentage_time);
        }
    }

    //Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
            printf("| %s | Deleted\n", start_array[i].pcTaskName);
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            printf("| %s | Created\n", end_array[i].pcTaskName);
        }
    }
    ret = ESP_OK;

exit:    //Common return path
    free(start_array);
    free(end_array);
    return ret;
}

