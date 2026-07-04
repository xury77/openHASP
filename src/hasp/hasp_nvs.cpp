/* MIT License - Copyright (c) 2019-2026 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ESP32

#include "hasplib.h"
#include "hasp_nvs.h"

#include "esp_idf_version.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

bool nvs_user_begin(Preferences& preferences, const char* key, bool readonly)
{
    if(preferences.begin(key, false, "config")) {
        LOG_DEBUG(TAG_NVS, "config partition opened");
        return true;
    }
    if(preferences.begin(key, false)) {
        LOG_WARNING(TAG_NVS, "config partition not found");
        return true;
    }
    LOG_ERROR(TAG_NVS, "Failed to open partition");
    return false;
}

bool nvs_clear_user_config()
{
    static const char* name[] = {FP_TIME, FP_OTA, FP_HTTP, FP_FTP, FP_MQTT, FP_WIFI, FP_WG};
    Preferences preferences;
    bool state = true;

    for(int i = 0; i < ARRAY_SIZE(name); i++) {
        if(preferences.begin(name[i], false) && !preferences.clear()) state = false;
        preferences.end();
    }

    for(int i = 0; i < ARRAY_SIZE(name); i++) {
        if(preferences.begin(name[i], false, "config") && !preferences.clear()) state = false;
        preferences.end();
    }

    return state;
}

bool nvsUpdateString(Preferences& preferences, const char* key, JsonVariant value)
{
    bool changed    = false;
    const char* val = value.as<const char*>();

    if(!value.isNull()) {                                            // Json key exists
        if(preferences.isKey(key)) {                                 // Nvs key exists
            changed = preferences.getString(key, "") != String(val); // Value changed
        } else
            changed = true; // Nvs key doesnot exist, create it
        if(changed) {
            size_t len = preferences.putString(key, val);
            LOG_DEBUG(TAG_NVS, F(D_BULLET "Wrote %s => %s (%d bytes)"), key, val, len);
        }
    }

    return changed;
}

bool nvsUpdateUInt(Preferences& preferences, const char* key, JsonVariant value)
{
    bool changed = false;
    uint32_t val = value.as<uint32_t>();

    if(!value.isNull()) {                                 // Json key exists
        if(preferences.isKey(key)) {                      // Nvs key exists
            changed = preferences.getUInt(key, 0) != val; // Value changed
        } else
            changed = true; // Nvs key doesnot exist, create it
        if(changed) {
            size_t len = preferences.putUInt(key, val);
            LOG_DEBUG(TAG_TIME, F(D_BULLET "Wrote %s => %d"), key, val);
        }
    }

    return changed;
}

bool nvsUpdateUShort(Preferences& preferences, const char* key, JsonVariant value)
{
    bool changed = false;
    uint32_t val = value.as<uint32_t>();

    if(!value.isNull()) {                                   // Json key exists
        if(preferences.isKey(key)) {                        // Nvs key exists
            changed = preferences.getUShort(key, 0) != val; // Value changed
        } else
            changed = true; // Nvs key doesnot exist, create it
        if(changed) {
            size_t len = preferences.putUShort(key, val);
            LOG_DEBUG(TAG_TIME, F(D_BULLET "Wrote %s => %d"), key, val);
        }
    }

    return changed;
}

// Show_nvs_keys.ino
// Read all the keys from nvs partition and dump this information.
// 04-dec-2017, Ed Smallenburg.
//
#include <stdio.h>
#include <string.h>
#include <esp_partition.h>
#include <nvs.h>
// Debug buffer size
#define DEBUG_BUFFER_SIZE 130
#define DEBUG true

struct nvs_entry
{
    uint8_t Ns;    // Namespace ID
    uint8_t Type;  // Type of value
    uint8_t Span;  // Number of entries used for this item
    uint8_t Rvs;   // Reserved, should be 0xFF
    uint32_t CRC;  // CRC
    char Key[16];  // Key in Ascii
    uint64_t Data; // Data in entry
};

struct nvs_page // For nvs entries
{               // 1 page is 4096 bytes
    uint32_t State;
    uint32_t Seqnr;

    uint32_t Unused[5];
    uint32_t CRC;
    uint8_t Bitmap[32];
    nvs_entry Entry[126];
};

//**************************************************************************************************
//                                          D B G P R I N T                                        *
//**************************************************************************************************
// Send a line of info to serial output.  Works like vsprintf(), but checks the DEBUG flag.        *
// Print only if DEBUG flag is true.  Always returns the formatted string.                         *
//**************************************************************************************************
char* dbgprint(const char* format, ...)
{
    static char sbuf[DEBUG_BUFFER_SIZE]; // For debug lines
    va_list varArgs;                     // For variable number of params

    va_start(varArgs, format);                      // Prepare parameters
    vsnprintf(sbuf, sizeof(sbuf), format, varArgs); // Format the message
    va_end(varArgs);                                // End of using parameters
    if(DEBUG)                                       // DEBUG on?
    {
        Serial.print("D: ");  // Yes, print prefix
        Serial.println(sbuf); // and the info
    }
    return sbuf; // Return stored string
}

void nvs_setup()
{
    /*
    // Example of listing all the key-value pairs of any type under specified partition and namespace
    nvs_iterator_t it = NULL;
    esp_err_t res     = nvs_entry_find("data", NULL, NVS_TYPE_ANY, &it);
    while(res == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
        printf("key '%s', type '%d' \n", info.key, info.type);
        res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    */

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    // Legacy ESP-IDF v4 implementation
    nvs_iterator_t it = nvs_entry_find("config", NULL, NVS_TYPE_ANY);
    nvs_entry_info_t info;

    while(it) {
        nvs_entry_info(it, &info);
        printf("%s::%s type=%d\n", info.namespace_name, info.key, info.type);
        it = nvs_entry_next(it);
    }
#else
    // Modern ESP-IDF v5 implementation
    nvs_iterator_t it = NULL;
    nvs_entry_info_t info;
    esp_err_t res = nvs_entry_find("nvs", "config", NVS_TYPE_ANY, &it); // Note: explicit partition + namespace

    while(res == ESP_OK && it != NULL) {
        nvs_entry_info(it, &info);
        printf("%s::%s type=%d\n", info.namespace_name, info.key, info.type);
        res = nvs_entry_next(&it); // Modifies 'it' in-place via pointer
    }
    
    if (it != NULL) {
        nvs_release_iterator(it);
    }
#endif

    // Example of nvs_get_stats() to get the number of used entries and free entries:
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    printf("Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n", nvs_stats.used_entries,
           nvs_stats.free_entries, nvs_stats.total_entries);

    { // TODO: remove migratrion of keys from default NVS partition to CONFIG partition
        const struct {
            const char* name;
            const char* config;
        } sec[] = {
            { .name = FP_TIME,  .config = FP_CONFIG_PASS },
            { .name = FP_OTA,   .config = FP_CONFIG_PASS },
            { .name = FP_HTTP,  .config = FP_CONFIG_PASS },
            { .name = FP_FTP,   .config = FP_CONFIG_PASS },
            { .name = FP_MQTT,  .config = FP_CONFIG_PASS },
            { .name = FP_WIFI,  .config = FP_CONFIG_PASS },
            { .name = FP_WG,    .config = FP_CONFIG_PRIVATE_KEY }
        };
        Preferences oldPrefs, newPrefs;

        for(int i = 0; i < ARRAY_SIZE(sec); i++) {
            if(oldPrefs.begin(sec[i].name, false) && newPrefs.begin(sec[i].name, false, "config")) {
                LOG_INFO(TAG_NVS, "opened %s", sec[i].name);
                String password = oldPrefs.getString(sec[i].config, D_PASSWORD_MASK);
                if(password != D_PASSWORD_MASK) {
                    LOG_INFO(TAG_NVS, "found %s %s => %s", sec[i].name, D_PASSWORD_MASK, password.c_str());
                    size_t len = newPrefs.putString(sec[i].config, password);
                    if(len == password.length()) {
                        oldPrefs.remove(sec[i].config);
                        LOG_INFO(TAG_NVS, "Moved %s key %s to new NVS partition", sec[i].name, sec[i].config);
                    }
                }
            }
            oldPrefs.end();
            newPrefs.end();
        }
    }
}

#endif