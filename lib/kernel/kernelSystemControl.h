#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <kernelDevice.h>
#include <Ticker.h>
#pragma region TIME
WiFiUDP ntpUDP;
AsyncWebServer server(80);
const int TIME_ZONE = -3;
const String TIME_NTP_SERVER = "br.pool.ntp.org";
NTPClient NTP_TIME_CLIENT(
    ntpUDP,
    TIME_NTP_SERVER.c_str(),
    TIME_ZONE * 3600,
    30000);
#pragma endregion TIME

typedef enum {
  ESP01S = 001,
  ESP12F = 012,
  ESP07 = 007,
  NODEMCU = 100,
  D1MINI = 101,
  D1MINIPRO = 102
} boardModel;

class kernelSystemControl {
 private:
  //##### GENERAL VAR . BEGIN #####
  const String COMPANY = "Tolfin System Control";
  const String FILE_CONFIG_NAME = "/config.json";
  const String FILE_LOG_NAME = "/log.txt";
  //##### GENERAL VAR . END #####

  struct config {
    int start = 0;
    String MAC;
    String CHIP_ID;
    uint32_t FLASH_SIZE;

    //String LoginUser;
    //String LoginPWD;

    String BoardName;
    String NetworkName;
    //String ServerType;
    //String ServerIP;
    //int serverPort;
    //String ServerUser;
    //String ServerPWD;
  } configLoad;

  void loadConfig() {
    DynamicJsonDocument doc(1536);
    JsonObject obj;
    obj = getFileFromObject(&doc, FILE_CONFIG_NAME);
    configLoad.start = obj["START"];

    configLoad.MAC = obj["MAC"].as<String>();
    configLoad.CHIP_ID = obj["CHIP"].as<String>();
    configLoad.FLASH_SIZE = obj["FLASH_SIZE"];

    configLoad.BoardName = obj["BOARD_NAME"].as<String>();
    configLoad.NetworkName = obj["NETWORK_NAME"].as<String>();
  }

//FILE SYSTEM
#pragma region FILE SYSTEM
  bool FILE_SYSTEM_OPEN = false;
  void openLittleFS(void) {
    // Abre o sistema de arquivos
    if (!LittleFS.begin()) {
      FILE_SYSTEM_OPEN = false;
      SerialPrint(SerialType::Error, "Erro ao abrir o sistema de arquivos");
    } else {
      FILE_SYSTEM_OPEN = true;
      SerialPrint(SerialType::Info, "Sistema de arquivos aberto com sucesso!");
    }
  }
  bool checkFileExist(String filename) {
    bool fileExist = LittleFS.exists(filename);
    return fileExist;
  }
  JsonArray getFileFromArray(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true) {
    File rFile = LittleFS.open(filename, "a+");
    if (rFile) {
      DeserializationError error = deserializeJson(*doc, rFile);
      if (error) {
        if (forceCleanONJsonError) {
          return doc->to<JsonArray>();
        }
      }
      rFile.close();
      return doc->as<JsonArray>();
    } else {
      return doc->to<JsonArray>();
    }
  }
  JsonObject getFileFromObject(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true) {
    File rFile = LittleFS.open(filename, "a+");
    if (rFile) {
      DeserializationError error = deserializeJson(*doc, rFile);
      if (error) {
        if (forceCleanONJsonError) {
          return doc->to<JsonObject>();
        }
      }
      rFile.close();
      return doc->as<JsonObject>();
    } else {
      return doc->to<JsonObject>();
    }
  }
  bool saveConfigFile(DynamicJsonDocument *doc) {
    if (saveJSonToAFile(doc, FILE_CONFIG_NAME)) {
      return true;
    } else {
      return false;
    }
  }
  bool saveJSonToAFile(DynamicJsonDocument *doc, String filename) {
    File rFile = LittleFS.open(filename, "w+");
    if (rFile) {
      serializeJson(*doc, rFile);
      rFile.close();
      return true;
    } else {
      return false;
    }
  }
#pragma endregion FILE SYSTEM

//LOG SYSTEM
#pragma region LOG SYSTEM
  enum SerialType {
    Log,
    Info,
    Warning,
    Error
  };
  void SerialPrintInternal(SerialType _type, const char *msg) {
    switch (_type) {
      case SerialType::Log:
        Serial.print("[LOG] ");
        break;
      case SerialType::Info:
        Serial.print("[INFO] ");
        break;
      case SerialType::Warning:
        Serial.print("[WARN] ");
        break;
      case SerialType::Error:
        Serial.print("[ERRO] ");
        break;
      default:
        Serial.print("[DEFA] ");
        break;
    }
    Serial.print(msg);
    //Not save "Log"
    if (_type != SerialType::Log) {
      writeFileLog(_type, msg);
    } else {
      Serial.print("    ");
      Serial.print("\t");
      Serial.print(">>WITHOUT SAVE");
    }
    Serial.println();
  }
  void writeFileLog(SerialType _type, String msg) {
    if (!FILE_SYSTEM_OPEN) {
      return;
    }
    char bufferDateTime[50] = "NO_DATE_TIME";
    if (WiFi.status() == WL_CONNECTED) {
      NTP_TIME_CLIENT.update();
      delay(50);
      time_t epochTime = NTP_TIME_CLIENT.getEpochTime();
      struct tm *ptm = gmtime((time_t *)&epochTime);
      String currentDate = String(ptm->tm_year + 1900) + "-" + String(ptm->tm_mon + 1) + "-" + String(ptm->tm_mday);
      String formattedTime = NTP_TIME_CLIENT.getFormattedTime();
      sprintf(bufferDateTime, "%s %s", currentDate.c_str(), formattedTime.c_str());
    }

    DynamicJsonDocument docConfig(2048);
    JsonObject objConfig;
    objConfig = getFileFromObject(&docConfig, FILE_CONFIG_NAME);

    String bufferType;
    if (_type == SerialType::Log) {
      bufferType = "LOG";
    } else if (_type == SerialType::Info) {
      bufferType = "INFO";
    } else if (_type == SerialType::Warning) {
      bufferType = "WARN";
    } else if (_type == SerialType::Error) {
      bufferType = "ERROR";
    } else {
      bufferType = "DEFAULT";
    }

    char bufferLog[1024];
    sprintf(bufferLog, "{\"datetime\":\"%s\",\"start\":%s,\"millis\":%s,\"type\":\"%s\",\"message\":\"%s\"},", bufferDateTime, objConfig["START"].as<String>().c_str(), String(millis()).c_str(), bufferType.c_str(), msg.c_str());

    Serial.print("    ");
    Serial.print("\t");
    File rFile = LittleFS.open(FILE_LOG_NAME, "a+");
    if (rFile) {
      rFile.println(bufferLog);
      Serial.print(">>SAVE OK (");
      Serial.print(rFile.size());
      Serial.print(")");
      rFile.close();
    } else {
      Serial.print(">>ERROR SAVE FILE");
    }
  }

  void SerialPrint(SerialType _type, const String msg, ...) {
    char buffer[300];
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsprintf(buffer, msg.c_str(), arg_ptr);
    va_end(arg_ptr);
    SerialPrintInternal(_type, buffer);
  }

  void SerialPrint(SerialType _type, const char *msg) {
    SerialPrintInternal(_type, msg);
  }
#pragma endregion LOG SYSTEM

//WIFI MANAGER
#pragma region WIFI
  WiFiClient espClient;
  PubSubClient clientMQTT;
  const char *WlStatusToStr(wl_status_t wlStatus) {
    switch (wlStatus) {
      case WL_NO_SHIELD:
        return "WL_NO_SHIELD";
      case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
      case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
      case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
      case WL_CONNECTED:
        return "WL_CONNECTED";
      case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
      case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
      case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
      default:
        return "Unknown";
    }
  }
  void WiFiStationSetup(void) {
    WiFi.disconnect();
    delay(10);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    delay(10);
    WiFi.hostname(configLoad.NetworkName.c_str());
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setPhyMode(WIFI_PHY_MODE_11B);
    //WiFi.config( IPAddress( 192,168,4,70 ), IPAddress( 192,168,4,1 ), IPAddress( 255,255,255,0 ) );
    SerialPrint(SerialType::Info, "WIFI SETUP - OK");
  }

  int WIFI_INTERVAL = 9000;
  uint32_t WIFI_Time = 0;
  uint32_t WIFI_StartTime = 0;
  int32_t WIFI_TimeLapsed = 0;

  void monitorWiFi() {
    if (WIFI_StartTime > millis()) {
      WIFI_StartTime = UINT32_MAX - (UINT32_MAX - millis());
    } else {
      WIFI_StartTime = millis();
    }

    if (WIFI_StartTime > (WIFI_Time + WIFI_INTERVAL)) {
      WIFI_Time = WIFI_StartTime;
      WiFiState();
    }
    WIFI_TimeLapsed = (int32_t)millis() - WIFI_StartTime;
  }
  wl_status_t wifi_status;
  wl_status_t wifi_status_last;

  bool WiFi_Connected = false;
  bool WiFi_Disconnected = false;
  int WiFi_Connection_Time_Out = 20;
  //
  int WiFi_Connection_Time = 0;
  int connectMQTTReplay = 0;

  Ticker deferred;
  void serverConfig() {
    server.on("/", HTTP_GET, [=](AsyncWebServerRequest *request) {
      DynamicJsonDocument doc(512);
 
      doc["boardName"] = configLoad.BoardName;
      doc["HN"] = String(WiFi.getHostname());
      doc["IP"] = WiFi.localIP().toString();
      doc["GW"] = WiFi.gatewayIP().toString();
      doc["NM"] = WiFi.subnetMask().toString();
      doc["SG"] = WiFi.RSSI();
      doc["chipId"] = ESP.getChipId();
      doc["flashChipId"] = ESP.getFlashChipId();
      doc["flashChipSize"] = ESP.getFlashChipSize();
      doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
      doc["freeHeap"] = ESP.getFreeHeap();

      String buf;
      serializeJson(doc, buf);
      request->send(200, F("application/json"), buf);
    });
    server.on("/log", HTTP_GET, [=](AsyncWebServerRequest *request) {
      request->send(LittleFS, FILE_LOG_NAME, String(), false);
    });
    server.on("/reset", HTTP_GET, [=](AsyncWebServerRequest *request) {
      if(!request->authenticate("tolfin", "12124545"))
        return request->requestAuthentication();
      LittleFS.remove(FILE_LOG_NAME); 
      LittleFS.remove(FILE_CONFIG_NAME); 
      request->send(200, "text/plain", "RESET FILES - OK");
      deferred.once_ms(700, []() {
        ESP.restart();
      });
    });
    server.on("/restart", HTTP_GET, [=](AsyncWebServerRequest *request) {
      if(!request->authenticate("tolfin", "12124545"))
        return request->requestAuthentication();
      request->send(200, "text/plain", "RESTARTING SYSTEM - OK");
      deferred.once_ms(700, []() {
        ESP.restart();
      });
    });
    server.begin();
  }

  void WiFiState() {
    wifi_status = WiFi.status();
    if (wifi_status_last != wifi_status) {
      SerialPrint(SerialType::Info, "CONNECTION STATUS: %s", WlStatusToStr(wifi_status));
      wifi_status_last = wifi_status;
    }
    switch (wifi_status) {
      case WL_CONNECTED:
        if (!WiFi_Connected) {
          WiFi_Connected = true;
          SerialPrint(SerialType::Info, "WIFI CONNECTED SSID: %s , IP: %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
          connectOTA();
          serverConfig();
        }
        if (WiFi_Connected) {
          if (MQTTConfigured && !clientMQTT.connected() && connectMQTTReplay == 0) {
            //Nao mexer
            connectMQTTReplay = 1;
            connectMQTT();
          } else if (connectMQTTReplay != 0) {
            connectMQTTReplay = 0;
          }
        }
        WiFi_Connection_Time = 0;

        break;
      case WL_CONNECTION_LOST:
        SerialPrint(SerialType::Error, "WIFI CONNECTION LOST !");
        WiFiStationStart();
        break;
      case WL_NO_SSID_AVAIL:
        SerialPrint(SerialType::Error, "WIFI CONNECTION FAIL - WL_NO_SSID_AVAIL");
        WiFiStationStart();
        break;
      case WL_CONNECT_FAILED:
        SerialPrint(SerialType::Error, "WIFI CONNECTION FAIL - WL_CONNECT_FAILED");
        WiFiStationStart();
        break;
      case WL_DISCONNECTED:
        WiFi_Connected = false;
        if (WiFi_Connection_Time >= WiFi_Connection_Time_Out) {
          WiFi_Connection_Time = 0;
          WiFiStationStart();
        } else
          WiFi_Connection_Time += 5;
        break;
      case WL_IDLE_STATUS:
        if (WiFi_Connected) {
          WiFi_Disconnected = true;
          WiFiStationSetup();
          delay(1000);
        }
        WiFiStationStart();
        break;
      default:  //
        SerialPrint(SerialType::Error, "WIFI CONNECTION FAIL - UNKNOWN");
        WiFiStationSetup();
        delay(1000);
        WiFiStationStart();
        break;
    }
  }
  void WiFiStationStart(void) {
    WiFi_Connected = false;
    String ssid = wifiList[wifiListConnectActual].SSID;
    String pwd = wifiList[wifiListConnectActual].PWD;
    WiFi.begin(ssid, pwd.c_str());
    SerialPrint(SerialType::Info, "TRY CONNECT WIFI: %s", ssid.c_str());
    ++wifiListConnectActual;
    if (wifiListConnectActual >= wifiListCount) {
      wifiListConnectActual = 0;
    }
  }
  struct WifiList {
    String SSID;
    String PWD;
  };
  WifiList wifiList[10];
  int wifiListCount = 0;
  int wifiListConnectActual = 0;

  bool OTAConfigured = false;

  bool MQTTConfigured = false;
  IPAddress MQTT_Domain;
  uint16_t MQTT_Port;
  const char *MQTT_User;
  const char *MQTT_Pass;
  const char *MQTT_Availability;

  void connectOTA() {
    if (OTAConfigured) {
      ArduinoOTA.onStart([]() {
        Serial.println("Inicio...");
      });
      ArduinoOTA.onEnd([]() {
        Serial.println("nFim!");
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Erro [%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Autenticacao Falhou");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Falha no Inicio");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Falha na Conexao");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Falha na Recepcao");
        else if (error == OTA_END_ERROR)
          Serial.println("Falha no Fim");
      });
      ArduinoOTA.begin();
    }
  }

  String IpAddress2String(const IPAddress &ipAddress) {
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
  }

  std::function<bool(const char *, const char *, boolean)> publishSend = [=](const char *topic, const char *payload, boolean retained) {
    if (mqttState()) {
      return clientMQTT.publish(topic, payload, retained);
    } else {
      return false;
    }
  };

  void connectMQTT() {
    if (MQTTConfigured) {
      if (!clientMQTT.connected()) {
        SerialPrint(SerialType::Info, "TRY CONNECTION MQTT... %i", clientMQTT.state());
        if (clientMQTT.connect(configLoad.NetworkName.c_str(), MQTT_User, MQTT_Pass, MQTT_Availability, 1, true, "offline")) {
          clientMQTT.subscribe("homeassistant/status");
          clientMQTT.publish(MQTT_Availability, "online", true);
          for (int i = 0; i < currentDeviceCount; i++) {
            devices[i]->setClient(publishSend);
            if (devices[i]->getReceiver() != NULL) {
              clientMQTT.subscribe(devices[i]->getReceiver());
            }
            if (devices[i]->callConnect != NULL) {
              devices[i]->callConnect(false);
            }
            //devices[i]->sendStatus();
          }
          SerialPrint(SerialType::Info, "MQTT CONNECTED!");
        } else {
          SerialPrint(SerialType::Error, "MQTT CONNECT FAIL - %i", clientMQTT.state());
        }
      }
    }
  }

  std::function<void(char *, uint8_t *, unsigned int)> callbackCustomMQTT = [=](char *topic, byte *payload, unsigned int length) {
    char message[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      message[i] = (char)payload[i];
    }
    message[length] = '\0';
    if (strcmp("homeassistant/status", topic) == 0) {
      String messageReceive = message;
      if (messageReceive == "online") {
        clientMQTT.publish(MQTT_Availability, "online", true);
        for (int i = 0; i < currentDeviceCount; i++) {
          if (devices[i]->callConnect != NULL) {
            devices[i]->callConnect(true);
          }
        }
      }
    } else {
      for (int i = 0; i < currentDeviceCount; i++) {
        if (devices[i]->getReceiver() != NULL) {
          if (strcmp(devices[i]->getReceiver(), topic) == 0) {
            devices[i]->receiver(message);
          }
        }
      }
    }
  };
#pragma endregion WIFI

 public:
  void init(String identificationBoard, boardModel _modelMicroController, uint8_t _ledStatus = -1) {
    openLittleFS();

    SerialPrint(SerialType::Info, "Booting Board, Wait...");

    rst_info *resetInfo;
    resetInfo = ESP.getResetInfoPtr();
    if (resetInfo->reason != 0){
      char buffer[40];
      sprintf(buffer, "Fatal Reboot Detect: %d", resetInfo->reason);
      SerialPrint(SerialType::Info, buffer);
    }

    DynamicJsonDocument doc(1200);
    JsonObject obj = getFileFromObject(&doc, FILE_CONFIG_NAME);
    if (obj.containsKey("START")) {
      obj["START"] = (int)obj["START"] + 1;
    } else {
      obj["START"] = 0;
    }
    if ((int)obj["START"] == 0) {
      SerialPrint(SerialType::Info, "First time executing, Wait...");
      obj["MAC"] = WiFi.macAddress();
      char out[20];
      sprintf(out, "%X", ESP.getChipId());
      obj["CHIP"] = out;
      obj["FLASH_SIZE"] = ESP.getFlashChipRealSize();

      //Settings
      obj["BOARD_NAME"] = identificationBoard;
      obj["NETWORK_NAME"] = String("BOARD_" + identificationBoard);

      saveConfigFile(&doc);
      SerialPrint(SerialType::Info, "Restarting board, Wait...");
      delay(1000);
      ESP.restart();
    }
    saveConfigFile(&doc);
    delay(300);
    loadConfig();
    SerialPrint(SerialType::Log, "CHIP: %s", configLoad.CHIP_ID.c_str());
    SerialPrint(SerialType::Log, "MAC: %s", configLoad.MAC.c_str());
    delay(100);
    WiFiStationSetup();
  }

  void addNetwork(const char *SSID, const char *PWD) {
    wifiList[wifiListCount].SSID = SSID;
    wifiList[wifiListCount].PWD = PWD;
    ++wifiListCount;
  }

  void OTAConfig(const char *_pass = NULL) {
    OTAConfigured = true;
    
    if (_pass) {
      ArduinoOTA.setPassword(_pass);
    }
    ArduinoOTA.setHostname(configLoad.NetworkName.c_str());
    ArduinoOTA.setPort(37517);
  }

  void MQTTConfig(IPAddress domain, uint16_t port, const char *user, const char *pwd, const char *availability) {
    MQTT_Domain = domain;
    MQTT_Port = port;
    MQTTConfigured = true;
    MQTT_User = user;
    MQTT_Pass = pwd;
    MQTT_Availability = availability;
    clientMQTT.setClient(espClient);
    clientMQTT.setServer(domain, port);
    clientMQTT.setCallback(callbackCustomMQTT);
  }

  kernelDevice *devices[12] = {};
  uint8_t currentDeviceCount = 0;
  uint8_t addDevice(kernelDevice *d) {
    devices[currentDeviceCount] = d;
    return ++currentDeviceCount;
  }

  bool mqttState() {
    return clientMQTT.connected();
  }

  bool sendMessage(const char *topic, const char *message) {
    if (mqttState()) {
      return clientMQTT.publish(topic, message);
    } else {
      return false;
    }
  }
  
  void Service() {
    clientMQTT.loop();
    ArduinoOTA.handle();
    this->monitorWiFi();
  }
};

kernelSystemControl baseSystemControl;