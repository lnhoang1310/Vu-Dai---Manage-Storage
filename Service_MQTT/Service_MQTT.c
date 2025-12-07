#include "Service_MQTT.h"
#include "esp_log.h"
#include "cJSON.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "MQTT_SERVICE";

Response_MQTT mqtt_response = {0};
static esp_mqtt_client_handle_t mqtt_client = NULL;

// ------------------------ Parse JSON -------------------------
static void MQTT_ParseJSON(const char *json)
{
    memset(&mqtt_response, 0, sizeof(mqtt_response));

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        ESP_LOGE(TAG, "JSON ERROR!");
        return;
    }

    // JSON dạng chuỗi: "Card Exist"
    if (cJSON_IsString(root))
    {
        const char *msg = root->valuestring;

        if (strcmp(msg, "Card Exist") == 0)
        {
            mqtt_response.exist = true;
            mqtt_response.valid = false;
        }
        else if (strcmp(msg, "Card not exist") == 0)
        {
            mqtt_response.exist = false;
            mqtt_response.valid = false;
        }

        ESP_LOGW(TAG, "Server Message: %s", msg);
        cJSON_Delete(root);
        return;
    }

    // JSON dạng object
    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *weight = cJSON_GetObjectItem(root, "weight");
    cJSON *time = cJSON_GetObjectItem(root, "time");

    if (cJSON_IsString(name) && cJSON_IsNumber(weight) && cJSON_IsString(time))
    {

        strcpy(mqtt_response.name, name->valuestring);
        mqtt_response.weight = weight->valuedouble;
        strcpy(mqtt_response.time, time->valuestring);

        mqtt_response.valid = true;
        mqtt_response.exist = true;

        ESP_LOGI(TAG, "JSON OK: name=%s, weight=%.2f, time=%s",
                 mqtt_response.name, mqtt_response.weight, mqtt_response.time);
    }
    else
    {
        ESP_LOGE(TAG, "JSON missing fields");
    }

    cJSON_Delete(root);
}

// ------------------------ Publish -------------------------
void MQTT_Publish(const char *topic, const char *id, const char *type)
{
    if (!mqtt_client)
        return;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", id);
    cJSON_AddStringToObject(root, "type", type);

    char *json = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(mqtt_client, topic, json, 0, 1, 0);

    ESP_LOGI(TAG, "Publish [%s]: %s", topic, json);

    free(json);
    cJSON_Delete(root);
}

// ------------------------ Event Handler -------------------------
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected");
        esp_mqtt_client_subscribe(mqtt_client, "storage/response", 1);
        break;

    case MQTT_EVENT_DATA:
    {
        int len = event->data_len;
        if (len > 500)
            len = 500;

        char data[512] = {0};
        memcpy(data, event->data, len);

        ESP_LOGI(TAG, "Received: %s", data);
        MQTT_ParseJSON(data);
    }
    break;

    default:
        break;
    }
}

// ------------------------ Init -------------------------
void MQTT_Init(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = "mqtt://192.168.1.10:1883",
        .credentials.client_id = "ESP32_Client",
    };
    mqtt_client = esp_mqtt_client_init(&cfg);

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);

    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "MQTT Started");
}
