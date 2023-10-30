#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WS2812FX.h>
enum class EspalexaColorMode {
  Switch,
  Power,
  brightness,
  RGB,
  RGBW,
  temperatureSensor
};

class Status {
 public:
  boolean state = 0;
  byte brightness = 0;
  byte red = 0;
  byte green = 0;
  byte blue = 0;
  byte white = 0;
  const char *loadEffect = NULL;
  String currentEffect = "Static";
  uint16_t fxspeed = 1000;
  double temp = 0;
};

typedef std::function<Status(Status oldStatus, Status new_last)> DeviceCallbackFunction;
typedef std::function<void(bool reconnect)> MQTTConnectionFunction;
typedef std::function<bool(const char *topic, const char *payload, boolean retained)> SendPublish;

struct Device {
  const char *uniqueID;
  const char *receiverTopic;
  const char *sendStateTopic;
  EspalexaColorMode type;
  DeviceCallbackFunction functionBack;
  MQTTConnectionFunction functionConnect;
  bool autoSend = true;
  bool checkLastMessage = true;
};

class kernelDevice {
 private:
  const char *uniqueID = "";

  const char *on_cmd = "ON";
  const char *off_cmd = "OFF";
  EspalexaColorMode type;

  const char *mqtt_receiver;
  const char *mqtt_status;
  boolean lock = 0;
  boolean autoSendStatus = 1;
  Status NewStatus = Status();
  Status OldStatus = Status();

  const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(20);

  bool checkLastMessage;
  String lastMessage = "";

 public:
  SendPublish clientMQTTdevice;
  DeviceCallbackFunction callBack = nullptr;
  MQTTConnectionFunction callConnect = nullptr;
  void disableAutoSend() {
    autoSendStatus = false;
  }

  void setLock() {
    lock = true;
  }

  struct Status getStatus() {
    return NewStatus;
  }

  void setStatus(Status status) {
    OldStatus = NewStatus;
    NewStatus = status;
  }

  bool sendStatus() {
    DynamicJsonDocument doc(BUFFER_SIZE);
    if (type == EspalexaColorMode::Switch) {
      doc["state"] = (NewStatus.state) ? on_cmd : off_cmd;
    }
    if (type == EspalexaColorMode::RGB) {
      doc["state"] = (NewStatus.state) ? on_cmd : off_cmd;
      JsonObject color = doc.createNestedObject("color");
      color["r"] = NewStatus.red;
      color["g"] = NewStatus.green;
      color["b"] = NewStatus.blue;
      doc["brightness"] = NewStatus.brightness;
      doc["effect"] = NewStatus.currentEffect;
      doc["speed"] = NewStatus.fxspeed;
      doc["color_mode"] = "rgb";
    }
    if (type == EspalexaColorMode::RGBW) {
      doc["state"] = (NewStatus.state) ? on_cmd : off_cmd;
      JsonObject color = doc.createNestedObject("color");
      color["r"] = NewStatus.red;
      color["g"] = NewStatus.green;
      color["b"] = NewStatus.blue;
      color["w"] = NewStatus.white;
      doc["brightness"] = NewStatus.brightness;
      doc["effect"] = NewStatus.currentEffect;
      doc["speed"] = NewStatus.fxspeed;
      doc["white_value"] = NewStatus.white;
      doc["color_mode"] = "rgbw";
    }
    if (type == EspalexaColorMode::brightness) {
      doc["state"] = (NewStatus.state) ? on_cmd : off_cmd;
      doc["brightness"] = NewStatus.brightness;
    }
    if (type == EspalexaColorMode::Power) {
      doc["state"] = (NewStatus.state) ? on_cmd : off_cmd;
    }
    if (type == EspalexaColorMode::temperatureSensor) {
      doc["temp"] = NewStatus.temp;
    }
    char buffer[150];
    serializeJson(doc, buffer);
    return clientMQTTdevice(mqtt_status, buffer, false);
  }

  const char *getReceiver() {

      return this->mqtt_receiver;

  }

  void setClient(SendPublish test) {
    clientMQTTdevice = test;
  }

  [[deprecated("Use Construct Device() class")]]
  kernelDevice(EspalexaColorMode _type, char *receiver, char *state, DeviceCallbackFunction call) {
    type = _type;
    mqtt_receiver = receiver;
    mqtt_status = state;
    callBack = call;
  }

  kernelDevice(Device _device) {
    if (_device.uniqueID != NULL) {
      this->uniqueID = _device.uniqueID;
    }
    if (_device.receiverTopic != NULL) {
      this->mqtt_receiver = _device.receiverTopic;
    }
    if (_device.sendStateTopic != NULL) {
      this->mqtt_status = _device.sendStateTopic;
    }
    this->type = _device.type;
    this->callBack = _device.functionBack;
    if (_device.functionConnect != NULL) {
      this->callConnect = _device.functionConnect;
    }
    this->autoSendStatus = _device.autoSend;
    this->checkLastMessage = _device.checkLastMessage;
  }

  void receiver(char *message) {
    if (lastMessage == String(message) && checkLastMessage) {
      return;
    } else {
      lastMessage = message;
    }
    OldStatus = NewStatus;
    DynamicJsonDocument doc(BUFFER_SIZE + 80);
    auto error = deserializeJson(doc, message);
    if (error) {
      return;
    }
    if (doc.containsKey("state")) {
      if (strcmp(doc["state"], on_cmd) == 0) {
        NewStatus.state = 1;
      } else if (strcmp(doc["state"], off_cmd) == 0) {
        NewStatus.state = 0;
      }
    }
    if (doc.containsKey("color")) {
      NewStatus.red = doc["color"]["r"];
      NewStatus.green = doc["color"]["g"];
      NewStatus.blue = doc["color"]["b"];
      if (doc["color"].containsKey("w")) {
        NewStatus.white = doc["color"]["w"];
      }
    }
    if (doc.containsKey("brightness")) {
      NewStatus.brightness = doc["brightness"];
    }
    if (doc.containsKey("effect")) {
      NewStatus.loadEffect = doc["effect"];
      NewStatus.currentEffect = NewStatus.loadEffect;
    }
    if (doc.containsKey("speed")) {
      NewStatus.fxspeed = doc["speed"];
    }
    if (lock) {
    } else {
      NewStatus = callBack(OldStatus, NewStatus);
      delay(10);
      if (autoSendStatus) {
        sendStatus();
      }
    }
  }
};