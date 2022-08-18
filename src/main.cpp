#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define PORTA_POTENCIOMETRO 15
#define BLE_SERVICE_UUID "f8ab3678-b2b6-11ec-b909-0242ac120002"
#define BLE_CHARACTERISTIC_TX_UUID "f8ab3a6a-b2b6-11ec-b909-0242ac120002" // Valor inteiro 
#define BLE_VELOCITY_CHARACTERISTIC_UUID "9a3843fe-ed19-11ec-8ea0-0242ac120002"

BLEServer *server = NULL;
BLECharacteristic *characteristicTx;
BLECharacteristic *velocityCharacteristic;

bool deviceConected = false;

int analog_value = 0;

bool oldDeviceConnected = false; //added

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pserver) {
      deviceConected = true;
  };

  void onDisconnect(BLEServer* pserver) {
      deviceConected = false;
  }
};

void checkToReconnect() //added
{
  // disconnected so advertise
  if (!deviceConected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    server->startAdvertising(); // restart advertising
    Serial.println("Disconnected: start advertising");
    oldDeviceConnected = deviceConected;
  }
  // connected so reset boolean control
  if (deviceConected && !oldDeviceConnected) {
    // do stuff here on connecting
    Serial.println("Reconnected");
    oldDeviceConnected = deviceConected;
  }
}


void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32-BLE");
  BLEServer *server = BLEDevice::createServer();
  server -> setCallbacks(new ServerCallbacks());

  BLEService *service = server -> createService(BLE_SERVICE_UUID);

  // Definindo characteristic para o service (teste)
  characteristicTx = service -> createCharacteristic(
    BLE_CHARACTERISTIC_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  characteristicTx -> addDescriptor(new BLE2902());

  // VelocityCharacteristic
  velocityCharacteristic = service -> createCharacteristic(
    BLE_VELOCITY_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  velocityCharacteristic -> addDescriptor(new BLE2902());

  service -> start();
  server -> getAdvertising() -> start();
}

void loop() {

  checkToReconnect();

  if (deviceConected) {
    analog_value = analogRead(PORTA_POTENCIOMETRO);
    int valor_mapeado = map(analog_value, 0, 4095, 0,100);

    char sensor_data[8];
    //char sensor_data2[] = { valor_mapeado };
    dtostrf(valor_mapeado, 3, 0, sensor_data);

    characteristicTx -> setValue(sensor_data);
    characteristicTx -> notify();

    // dummy velocidade/aceleracao
    float dummyVelocity = valor_mapeado * 0.01;
    char velocity_data[8];
    dtostrf(dummyVelocity, 1, 2, velocity_data);

    velocityCharacteristic -> setValue(velocity_data);
    velocityCharacteristic -> notify();
  }

  delay(500);
}

