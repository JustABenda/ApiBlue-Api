#include "./ConnectorBLE.hpp"

BLECharacteristic *ConnectorBLE::pCharacteristic;
std::string ConnectorBLE::vars = "";

int ConnectorBLE::Init(){
    BLEDevice::init("ESP32-BLE-Server.HIVE");
    BLEServer *pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(SERVICE_UUID);
    // vytvoření BLE komunikačního kanálu pro odesílání (TX)
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_TX_UUID,
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_READ
                        );
    // vytvoření BLE komunikačního kanálu pro příjem (RX)
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_RX_UUID,
                                            BLECharacteristic::PROPERTY_WRITE
                                        );
    pCharacteristic->setCallbacks(new MyCallbacks());
    // zahájení BLE služby
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    return 0;
}
int ConnectorBLE::SetData(std::string newdata){
    vars = newdata;
    return 0;
}
std::string ConnectorBLE::MakeCommand(char prefix, const char* args){
    return (prefix + (std::string)(args)).c_str();
}
char* ConnectorBLE::BuildData(){
    return "";
}
void ConnectorBLE::MyCallbacks::onWrite(BLECharacteristic *pCharacteristic){
    std::string value = pCharacteristic->getValue();
    char command = value.at(0);
    std::string data = value.substr(1, data.length());
    if (value.length() > 0)
    {
        if(command == REQUEST_CALL) SendData(MakeCommand(RESPONSE_CALL, ""));
        if(command == REQUEST_INFO) SendData(MakeCommand(RESPONSE_INFO, "ESP32, HIVE"));
        if(command == REQUEST_WARE) SendData(MakeCommand(RESPONSE_WARE, "1.0"));
        if(command == REQUEST_DATA) SendData(MakeCommand(RESPONSE_DATA, "Temperature_in:25.5,25.3,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.3,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5;Temperature_out:23.5,22.3,23.5,22.5,23.5,22.5,23.5,23.5,22.5,23.5,22.5,23.5,23.5,22.3,23.5,22.5,23.5,22.5,23.5,23.5,22.5,23.5,22.5,23.5;Humidity_in:25.5,25.3,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.3,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5,25.5;Humidity_out:23.5,22.3,23.5,22.5,23.5,22.5,23.5,23.5,22.5,23.5,22.5,23.5,23.5,22.3,23.5,22.5,23.5,22.5,23.5,23.5,22.5,23.5,22.5,23.5;Battery_in:0,15,30,45,60,70,80,30,45,60,70,80,0,15,30,45,60,70,80,30,45,60,70,80"));
        if(command == REQUEST_VARS) SendData(MakeCommand(RESPONSE_VARS, vars.c_str()));
        Serial.println("*********");
        Serial.print("New value: ");
        Serial.println(value.c_str());
        Serial.println("*********");
    }
}
void ConnectorBLE::SendData(std::string data){
    if(data.at(0) == REQUEST_DATA && data.length() > 353){
        std::string raw_data = data.substr(1, data.length());
        int count = data.length() / 350;
        float rest = data.length() - (count*350);
        if(rest > 0) count++;
        SendData(MakeCommand(RESPONSE_DATA, ((std::string)"00" + std::to_string(count)).c_str()));
        for(int i = 1; i <= count; i++){
            std::stringstream stream;
            stream << std::hex << i;
            std::string index = stream.str();
            index = index.length() > 1 ? index : "0" + index;
            //Serial.println(index.c_str());
            if(i == count){ //Send last message
                SendData(MakeCommand(RESPONSE_DATA, (index + raw_data.substr((i-1)*350, (int)rest)).c_str()));
            }else{ //Send common message
                SendData(MakeCommand(RESPONSE_DATA, (index + raw_data.substr((i-1)*350, 350)).c_str()));
            }
        }
    }else{
        pCharacteristic->setValue(data);
        pCharacteristic->notify();
    }
}