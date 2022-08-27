#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define PORTA_POTENCIOMETRO 15

#define BLE_SERVICE_UUID "f8ab3678-b2b6-11ec-b909-0242ac120002"

#define BLE_WEIGHT_CHARACTERISTIC_UUID "f8ab3a6a-b2b6-11ec-b909-0242ac120002" 
#define BLE_VELOCITY_CHARACTERISTIC_UUID "9a3843fe-ed19-11ec-8ea0-0242ac120002"
#define BLE_ACCELERATION_CHARACTERISTIC_UUID "d89f2437-86c0-4d8c-9c21-8a39e600827d"
#define BLE_OFFSET_CHARACTERISTIC_UUID "cf4e2566-14a1-4d71-84ad-96eebd5b9bc3"
#define BLE_FORCE_CHARACTERISTIC_UUID "e69526fa-8886-43da-a63a-25e8ed126b94"
#define BLE_POWER_CHARACTERISTIC_UUID "69454f3f-f575-4f59-87dd-42ce3207ddbf"

BLEServer *server = NULL;

BLECharacteristic *weightCharacteristic;
BLECharacteristic *velocityCharacteristic;
BLECharacteristic *accelerationCharacteristic;
BLECharacteristic *offsetCharacteristic;
BLECharacteristic *forceCharacteristic;
BLECharacteristic *powerCharacteristic;

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

  // WeightCharacteristic
  weightCharacteristic = service -> createCharacteristic(
    BLE_WEIGHT_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  weightCharacteristic -> addDescriptor(new BLE2902());

  // VelocityCharacteristic
  velocityCharacteristic = service -> createCharacteristic(
    BLE_VELOCITY_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  velocityCharacteristic -> addDescriptor(new BLE2902());

  // AccelerationCharacteristic
  accelerationCharacteristic = service -> createCharacteristic(
    BLE_ACCELERATION_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  accelerationCharacteristic -> addDescriptor(new BLE2902());

  // OffsetCharacteristic
  offsetCharacteristic = service -> createCharacteristic(
    BLE_OFFSET_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  offsetCharacteristic -> addDescriptor(new BLE2902());

  //ForceCharacteristic
  forceCharacteristic = service -> createCharacteristic(
    BLE_FORCE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  forceCharacteristic -> addDescriptor(new BLE2902());

  //PowerCharacteristic
  powerCharacteristic = service -> createCharacteristic(
    BLE_POWER_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  powerCharacteristic -> addDescriptor(new BLE2902());

  service -> start();
  server -> getAdvertising() -> start();
}

void loop() {

  checkToReconnect();

  if (deviceConected) {

    // Envio de Peso para o Bluetooth -> Valor do tipo int
    analog_value = analogRead(PORTA_POTENCIOMETRO);
    int valor_mapeado = map(analog_value, 0, 4095, 0,100);

    char sensor_data[8]; //char sensor_data2[] = { valor_mapeado };
    dtostrf(valor_mapeado, 3, 0, sensor_data);

    weightCharacteristic -> setValue(sensor_data);
    weightCharacteristic -> notify();

    // Envio da Velocidade no BLE -> Valor do tipo float
    float dummyVelocity = valor_mapeado * 0.01;

    char velocity_data[8];
    dtostrf(dummyVelocity, 1, 2, velocity_data);

    velocityCharacteristic -> setValue(velocity_data);
    velocityCharacteristic -> notify();

    // Envio da Aceleração no BLE -> Valor do tipo float
    float dummyAcceleration = valor_mapeado * 0.01;

    char accelerationData[8];
    dtostrf(dummyAcceleration, 1, 2, accelerationData);

    accelerationCharacteristic -> setValue(accelerationData);
    accelerationCharacteristic -> notify();

    // Envio da Posição no BLE -> Valor do tipo Int 
    offsetCharacteristic -> setValue(sensor_data); // Estou enviando os dados do potenciometro para teste
    offsetCharacteristic -> notify();

    // Envio da Força no BLE -> Valor do tipo Int
    float dummyForce = valor_mapeado * 2;

    char forceData[8];
    dtostrf(dummyForce, 3, 0, forceData);

    forceCharacteristic -> setValue(forceData);
    forceCharacteristic -> notify();

    // Envio da Potência no BLE -> Valor do tipo Int
    float dummyPower = valor_mapeado * 5;
    
    char powerData[8];
    dtostrf(dummyPower, 3, 0, powerData);

    powerCharacteristic -> setValue(powerData);
    powerCharacteristic -> notify();
  }

  delay(500);
}

