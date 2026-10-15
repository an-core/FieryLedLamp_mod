// *************************************************************************** Mqtt.h *******************************************************************
#pragma once
// --------------

#if USE_MQTT
#include <AsyncMqttClient.h>
#include "pgmspace.h"
#include "Constants.h"
#include "Types.h"
#include "Extern.h"
#include "Prototypes.h"
#include "WiFi.h"
// -----------------------------

static const char TopicCmnd[] PROGMEM = "cmnd";
static const char TopicState[] PROGMEM = "state";
static const char TopicSnd[] PROGMEM = "snd";
static const char MqttClientIdPrefix[] PROGMEM = "LedLamp_";
static const uint16_t MqttPort = 1883U;
static const size_t MQTT_INPUT_MAX_LEN = 254;

class Mqtt {
  public:
    static Mqtt& instance() {
      static Mqtt instance;
      return instance;
    }

    void begin(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate);
    void releaseMemory();
    void loop();
    void publishState(uint8_t flag = 0);

    bool isConnected() const {
      return mqttClient && mqttClient->connected();
    }
    const char* getClientId() const {
      return clientId;
    }

    bool publish(const char* topic, const char* value);
    bool needToPublish = false;
    char mqttBuffer[255] = {0};

  private:
    Mqtt() = default;
    ~Mqtt() = default;

    Mqtt(const Mqtt&) = delete;
    Mqtt& operator=(const Mqtt&) = delete;

    void mqttConnect();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    bool allocStr(char** str, const char* src);
    char* byteToHex(char* out, uint8_t value);

    AsyncMqttClient* mqttClient = nullptr;
    char* lampInputBuffer = nullptr;
    SendCurrentDelegate sendCurrentDelegate = nullptr;

    char* clientId = nullptr;
    char* topicInput = nullptr;
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
    char* topicOutput = nullptr;
#endif
    char* topicOutputJSON = nullptr;

    uint32_t mqttLastConnectingAttempt = 0;

    static const uint8_t qos = 0;
    static const uint32_t connectionTimeout = MQTT_RECONNECT_TIME * 1000U;
}; // class Mqtt

#endif // USE_MQTT

// ****************************************************************************************************************************************************
