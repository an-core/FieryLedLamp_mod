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
static const size_t MQTT_INPUT_MAX_LEN = 254; // максимальный размер входящего MQTT-сообщения (без учёта '\0')

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

// -------------------------------------------------------------------------------

void Mqtt::begin(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate) {
  this->mqttClient = mqttClient;
  this->lampInputBuffer = lampInputBuffer;
  this->sendCurrentDelegate = sendCurrentDelegate;

  if (mqttIPaddr)
    this->mqttClient->setServer(MqttServer, MqttPort);
  else
    this->mqttClient->setServer(MqttHost, MqttPort);

  // формирование clientId
  static const size_t prefixLen = sizeof(MqttClientIdPrefix) - 1; // 8
  char clientIdBuf[prefixLen + 8 + 1]; // +1 для '\0'
  strcpy_P(clientIdBuf, MqttClientIdPrefix);

#ifdef ESP32_USED
  uint32_t chipId = get_Chip_ID();
#else
  uint32_t chipId = ESP.getChipId();
#endif

  for (uint8_t i = 0; i < 4; ++i) {
    byteToHex(&clientIdBuf[i * 2 + prefixLen], chipId >> ((3 - i) * 8));
  }
  clientIdBuf[prefixLen + 8] = '\0';

  allocStr(&clientId, clientIdBuf);
  if (!clientId) {
#if MQTT_LOG
    SYSLOG.add("MQTT: не удалось выделить память для clientId");
#endif
    return;
  }
  this->mqttClient->setClientId(clientId);

  if (MqttUser[0] != '\0')
    this->mqttClient->setCredentials(MqttUser, MqttPassword);

  // Топики
  if (topicInput) { free(topicInput); topicInput = nullptr; }
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
  if (topicOutput) { free(topicOutput); topicOutput = nullptr; }
#endif
  if (topicOutputJSON) { free(topicOutputJSON); topicOutputJSON = nullptr; }

  // input: TopicBase/clientId/cmnd
  size_t topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + strlen(TopicCmnd) + 1;
  topicInput = (char*)malloc(topicLength);
  if (!topicInput) {
#if MQTT_LOG
    SYSLOG.add("MQTT: не удалось выделить topicInput (%u байт)", (unsigned)topicLength);
#endif
    return;
  }
  sprintf_P(topicInput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicCmnd);

#ifdef PUBLISH_STATE_IN_OLD_FORMAT
  topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + strlen(TopicState) + 1;
  topicOutput = (char*)malloc(topicLength);
  if (!topicOutput) {
#if MQTT_LOG
    SYSLOG.add("MQTT: не удалось выделить topicOutput (%u байт)", (unsigned)topicLength);
#endif
    return;
  }
  sprintf_P(topicOutput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicState);
#endif

  topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + strlen(TopicSnd) + 1;
  topicOutputJSON = (char*)malloc(topicLength);
  if (!topicOutputJSON) {
#if MQTT_LOG
    SYSLOG.add("MQTT: не удалось выделить topicOutputJSON (%u байт)", (unsigned)topicLength);
#endif
    return;
  }
  sprintf_P(topicOutputJSON, PSTR("%s/%s/%s"), TopicBase, clientId, TopicSnd);

#if MQTT_LOG
  SYSLOG.add("MQTT топик для входящих команд: %s", topicInput);
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
  SYSLOG.add("MQTT топик для выходных ответов лампы: %s", topicOutput);
#endif
  SYSLOG.add("MQTT топик для JSON-ответов: %s", topicOutputJSON);
#endif

  this->mqttClient->onConnect([this](bool sessionPresent) {
    onMqttConnect(sessionPresent);
  });
  this->mqttClient->onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
    onMqttDisconnect(reason);
  });
  this->mqttClient->onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    onMqttMessage(topic, payload, properties, len, index, total);
  });
}

// -------------------------------------------------------------------------------

void Mqtt::releaseMemory() {
  if (clientId) { free(clientId); clientId = nullptr; }
  if (topicInput) { free(topicInput); topicInput = nullptr; }
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
  if (topicOutput) { free(topicOutput); topicOutput = nullptr; }
#endif
  if (topicOutputJSON) { free(topicOutputJSON);topicOutputJSON = nullptr; }
}

// -------------------------------------------------------------------------------

void Mqtt::loop() {
  if (!MqttOn) return;
  if (!Wifi::instance().isConnected()) return;

  if (mqttClient && !mqttClient->connected()) {
    uint32_t now = millis();
    if (!mqttLastConnectingAttempt || (now - mqttLastConnectingAttempt >= connectionTimeout)) {
      mqttConnect();
    }
  }
}

// -------------------------------------------------------------------------------

void Mqtt::mqttConnect() {
#if MQTT_LOG
  SYSLOG.add("Подключение к MQTT брокеру \"");
  if (mqttIPaddr)
    SYSLOG.add(MqttServer);
  else
    SYSLOG.add(MqttHost);
  SYSLOG.add(':');
  SYSLOG.add(MqttPort);
  SYSLOG.add("\"...");
#endif
  mqttClient->disconnect();
  mqttClient->connect();
  mqttLastConnectingAttempt = millis();
}

// -------------------------------------------------------------------------------

bool Mqtt::publish(const char* topic, const char* value) {
  if (!mqttClient || !mqttClient->connected()) return false;
#if MQTT_LOG
  SYSLOG.add("Отправлено MQTT: топик \"");
  SYSLOG.add(topic);
  SYSLOG.add("\", значение \"");
  SYSLOG.add(value);
  SYSLOG.add('"');
#endif
  return mqttClient->publish(topic, qos, true, value, 0) != 0;
}

// -------------------------------------------------------------------------------

void Mqtt::onMqttConnect(bool sessionPresent) {
#if MQTT_LOG
  SYSLOG.add("Подключен к MQTT брокеру");
#endif
  mqttLastConnectingAttempt = 0;
  if (topicInput) {
    mqttClient->subscribe(topicInput, 1);
  }
  needToPublish = true;
}

// -------------------------------------------------------------------------------

void Mqtt::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
#if MQTT_LOG
  SYSLOG.add("Отключен от брокера MQTT");
#endif
}

// -------------------------------------------------------------------------------

void Mqtt::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (payload != NULL && lampInputBuffer != nullptr) {
    // Ограничиваем длину, чтобы не переполнить внешний буфер
    size_t copyLen = (len < MQTT_INPUT_MAX_LEN) ? len : MQTT_INPUT_MAX_LEN;
    memcpy(lampInputBuffer, payload, copyLen);
    lampInputBuffer[copyLen] = '\0';
    needToPublish = true;

#if MQTT_LOG
    SYSLOG.add("Получен MQTT: топик \"");
    SYSLOG.add(topic);
    SYSLOG.add("\", значение \"");
    SYSLOG.add(lampInputBuffer);
    SYSLOG.add("\"");
#endif
  }
}

// -------------------------------------------------------------------------------

void Mqtt::publishState(uint8_t flag) {
  if (!flag) {
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
    if (strlen(mqttBuffer) == 0)
      sendCurrentDelegate(mqttBuffer);

    if (strlen(mqttBuffer) > 0 && topicOutput) {
      publish(topicOutput, mqttBuffer);
      mqttBuffer[0] = '\0';
      needToPublish = false;
    }
#endif
  } else {
    if (topicOutputJSON) {
      publish(topicOutputJSON, mqttBuffer);
      mqttBuffer[0] = '\0';
      needToPublish = false;
    }
  }
}

// -------------------------------------------------------------------------------

char* Mqtt::byteToHex(char* out, uint8_t value) {
  uint8_t b = value >> 4;
  out[0] = (b < 10) ? '0' + b : 'A' + (b - 10);
  b = value & 0x0F;
  out[1] = (b < 10) ? '0' + b : 'A' + (b - 10);
  out[2] = '\0';
  return out;
}

// -------------------------------------------------------------------------------

bool Mqtt::allocStr(char** str, const char* src) {
  if (src && *src) {
    if (*str) {
      void* ptr = realloc(*str, strlen(src) + 1);
      if (!ptr) return false;
      *str = (char*)ptr;
    } else {
      *str = (char*)malloc(strlen(src) + 1);
      if (!*str) return false;
    }
    strcpy(*str, src);
  } else if (*str) {
    free(*str);
    *str = NULL;
  }
  return true;
}

// -------------------------------------------------------------------------------
#endif // USE_MQTT

// ****************************************************************************************************************************************************
