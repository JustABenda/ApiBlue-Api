#include <Arduino.h>
#include "./Api/BluetoothHandler/ConnectorBLE.hpp"
#include "./Api/Database/DatabaseHandler.hpp"
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Server");
  std::string vars[] = {"Temperature_out", "Temperature_in", "Humidity_in", "Mass_in", "Battery_in"};
  std::string units[] = {"°C", "°C", "%", "kg", "%"};
  std::string colors[] = {"orange", "red", "blue", "purple", "green"};
  DatabaseHandler::Init(vars, units, colors, 5);
  float values[] = {0,0,0,0,0};
  /*DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);
  DatabaseHandler::Log("2023-02-01 00:00", values);*/
  ConnectorBLE::Init();
  ConnectorBLE::SetData("Battery_in.%.green;Temperature_in.°C.red;Temperature_out.°C.orange;Humidity_in.%.blue;Humidity_out.%.aqua");
}

void loop() {
  Serial.println("Waiting for data");
  delay(5000);
}