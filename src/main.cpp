#include <Arduino.h>
#include <WS2812FX.h>
#include <kernelSystemControl.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>

#define GPIO_OUT_SW     12
#define GPIO_IN_STATUS  14 
#define GPIO_PWR_BTN    4
#define GPIO_ONEWIRE    0

#define OUT_STATE_ACTIVE    1
#define OUT_STATE_INACTIVE  (!OUT_STATE_ACTIVE)
#define OUT_TOGGLE_DURATION_MS  1000

#define IN_STATUS_INVERTED  true

boolean statusCurrent = NULL;

OneWire oneWire(GPIO_ONEWIRE);
DallasTemperature sensors(&oneWire);

void TogglePcState (void)
{
  digitalWrite (GPIO_OUT_SW, OUT_STATE_ACTIVE);
  delay (OUT_TOGGLE_DURATION_MS);
  digitalWrite (GPIO_OUT_SW, OUT_STATE_INACTIVE);
}

kernelDevice* mainDevice = new kernelDevice(Device{
    uniqueID: "powerComputer",
    receiverTopic: "powerComputer/set",
    sendStateTopic: "powerComputer/state",
    type: EspalexaColorMode::Power,
    functionBack: [](Status oldStatus, Status newStatus){
      TogglePcState();
      return newStatus;
    },
    autoSend: false,
    checkLastMessage: false
  });

kernelDevice* temperatureDevice = new kernelDevice(Device{
    uniqueID: "powerComputerTemperature",
    sendStateTopic: "powerComputer/temperature/state",
    type: EspalexaColorMode::temperatureSensor,
    autoSend: false
  });

void statusFunction(){
  if (!baseSystemControl.mqttState()){
    return;
  }

  boolean tmp_status = digitalRead(GPIO_IN_STATUS);
  if (IN_STATUS_INVERTED) tmp_status = !tmp_status;

  if (tmp_status != statusCurrent){
    statusCurrent = tmp_status;
    Status status = Status();
    status.state = tmp_status;
    mainDevice->setStatus(status);
    mainDevice->sendStatus();
  }
}

Ticker loopStatusCheck;

void setup() {
  Serial.begin(115200);
  
  pinMode (GPIO_OUT_SW, OUTPUT);
  digitalWrite (GPIO_OUT_SW, OUT_STATE_INACTIVE); 
  pinMode (GPIO_IN_STATUS, INPUT);
  pinMode (GPIO_PWR_BTN, INPUT);

  while (!Serial) delay(10);
  delay(500); // Without this delay the next print is not visible!

  sensors.begin();

  baseSystemControl.addDevice(mainDevice);
  baseSystemControl.addDevice(temperatureDevice);

  //Initial Paramters
  baseSystemControl.init("powerComputer", NODEMCU);

  //Set Wifi Networks
  baseSystemControl.addNetwork("SSID", "PASSWORD");

  //Enable OTA for UPDATES
  baseSystemControl.OTAConfig("PASSWORD_OTA");

  //Enable MQTT Connect
  baseSystemControl.MQTTConfig(IPAddress(192,168,15,1), 1883, "admin", "admin", "powerComputador/availability");

  loopStatusCheck.attach_ms(300, statusFunction);
}

long lastMsg = 0;
void loop() {
  baseSystemControl.Service();

  if (digitalRead(GPIO_PWR_BTN) == HIGH){
    DynamicJsonDocument doc(50);
    JsonObject root = doc.to<JsonObject>();
    root["device"] = "powerComputer";
    root["status"] = "ON";
    char buffer[100];
    serializeJson(doc, buffer);
    baseSystemControl.sendMessage("request/power",buffer);
    while (digitalRead(GPIO_PWR_BTN) == HIGH ){
      delay(200);
    }
  }

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    if (baseSystemControl.mqttState()){
      sensors.requestTemperatures(); 
      Status status = Status();
      status.temp = sensors.getTempCByIndex(0);
      temperatureDevice->setStatus(status);
      temperatureDevice->sendStatus();
    }
  }
}