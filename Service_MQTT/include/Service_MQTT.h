#ifndef SERVICE_MQTT_H
#define SERVICE_MQTT_H

#include <stdbool.h>

typedef struct {
    char name[64];
    float weight;
    char time[32];
    bool valid;          // true nếu parse JSON thành công
    bool exist;          // card exist hay không
} Response_MQTT;

extern Response_MQTT mqtt_response;

void MQTT_Init(void);
void MQTT_Publish(const char *topic, const char *id, const char *type);

#endif
