#include "BLEDevice.h"
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLERemoteCharacteristic.h>

// 定义 BLEClient 对象
BLEClient bleClient;

// 服务和特征 UUID
static BLEUUID serviceUUID("FFF0");
static BLEUUID charUUIDR("FFF1");
static BLEUUID charUUIDW("FFF2");

static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristicR;
static BLERemoteCharacteristic* pRemoteCharacteristicW;
static BLEAdvertisedDevice* myDevice;


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    private:
        std::string targetMacAddress;
    public:
        MyAdvertisedDeviceCallbacks(std::string macAddress) : targetMacAddress(macAddress) {}

      void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        // 将目标MAC地址和广告设备的MAC地址都转换为小写或大写进行比较
        std::transform(targetMacAddress.begin(), targetMacAddress.end(), targetMacAddress.begin(), ::tolower);
        std::string advertisedMacAddress = advertisedDevice.getAddress().toString();
        std::transform(advertisedMacAddress.begin(), advertisedMacAddress.end(), advertisedMacAddress.begin(), ::tolower);

        if (advertisedMacAddress == targetMacAddress) {
          BLEDevice::getScan()->stop();
          myDevice = new BLEAdvertisedDevice(advertisedDevice);
          Serial.println("Found our device! Stopping scan.");
        }
      }
};

bool GattClientConnect(std::string macAddress) {
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(macAddress));
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  while (myDevice == nullptr) {
    delay(100);
  }

  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");
  pClient->setMTU(517);

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristicR = pRemoteService->getCharacteristic(charUUIDR);
  if (pRemoteCharacteristicR == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUIDW.toString().c_str());
    pClient->disconnect();
    return false;
  }
  pRemoteCharacteristicW = pRemoteService->getCharacteristic(charUUIDW);
  if (pRemoteCharacteristicW == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUIDW.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  if (pRemoteCharacteristicR->canRead()) {
    std::string value = pRemoteCharacteristicR->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }
  connected = true;
  return true;
}

bool GattClientWrite(std::vector<uint8_t> data) {
  if (!connected || pRemoteCharacteristicW == nullptr) {
    return false;
  }
  pRemoteCharacteristicW->writeValue(data.data(), data.size());
  return true;
}

bool GattClientRead(std::vector<uint8_t>* data) {
  if (!connected || pRemoteCharacteristicR == nullptr) {
    return false;
  }
  std::string value = pRemoteCharacteristicR->readValue();
  data->assign(value.begin(), value.end());
  return true;
}
