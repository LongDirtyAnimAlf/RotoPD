#include "indicator_storage_nvs.h"
#include "nvs_flash.h"

//#define STORAGE_PARTITION "indicator"
#define STORAGE_NAMESPACE "settings"

esp_err_t indicator_nvs_init(void)
{
    esp_err_t ret;

    //const esp_partition_t *partitionNvs = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, STORAGE_PARTITION);
    //esp_err_t err = nvs_flash_init_partition_ptr(partitionNvs);    

    // ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    //esp_err_t ret = nvs_flash_init_partition(STORAGE_PARTITION);

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        //ESP_ERROR_CHECK(nvs_flash_erase_partition(STORAGE_PARTITION));
        //ret = nvs_flash_init_partition(STORAGE_PARTITION);        
    }
    return ret;    
}

esp_err_t IRAM_ATTR indicator_nvs_write(char *p_key, void *p_data, size_t len)
{
    static DRAM_ATTR nvs_handle_t my_handle;
    static DRAM_ATTR esp_err_t    err;

    //err = nvs_open_from_partition(STORAGE_PARTITION, STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);    
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_blob(my_handle, p_key, p_data, len);
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
    }

    nvs_close(my_handle);
    return err;
}

esp_err_t IRAM_ATTR indicator_nvs_read(char *p_key, void *p_data, size_t *p_len)
{
    static DRAM_ATTR nvs_handle_t my_handle;
    static DRAM_ATTR esp_err_t    err;

    //err = nvs_open_from_partition(STORAGE_PARTITION, STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);        
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_blob(my_handle, p_key, p_data, p_len);

    nvs_close(my_handle);
    return err;
}
