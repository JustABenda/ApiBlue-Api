#include "DatabaseHandler.hpp"

sqlite3 *DatabaseHandler::database;
sqlite3_stmt *DatabaseHandler::resource;
const char *DatabaseHandler::tail;
std::string *DatabaseHandler::varlist;
const char* DatabaseHandler::data = "Callback function called";
int DatabaseHandler::count = 0;
int DatabaseHandler::Init(std::string variables[], std::string units[], std::string colors[], int count){ //Run as first command
    Serial.println("Init Sequence...");
    DatabaseHandler::count = count;
    varlist = variables;
    if(!SPIFFS.begin()){Serial.println("SPIFFS Filesystem Error"); return -1;} //Check if filesystem exists
    if(!SPIFFS.exists("/data.db")){ //Ckecks if file exists
        Serial.println("Databse doesnt exist, creating one");
        File file = SPIFFS.open("/data.db", FILE_WRITE); //Create file
        if(!file){ //Check for file existance again
            Serial.println("Error creating databse file");
            return -1;
        }
        int setup = SetupDatabase(); //Create Databse structure
        if(setup != 0){Serial.println("Error during database setup");return setup;} //Throw error if setup failed
    }else int rc = sqlite3_open("/spiffs/data.db", &DatabaseHandler::database); //Select Database
    Serial.println("Init successfull");
    return 0;
}
int DatabaseHandler::SetupDatabase(){
    sqlite3_initialize();
    int rc = sqlite3_open("/spiffs/data.db", &DatabaseHandler::database); //Select Database
    if(rc) Serial.println("Unable to open database file");
    std::string command = "CREATE TABLE data ('datetime' TEXT";
    for(int i = 0; i < count; i++){
        command += ",'" + varlist[i] + "' FLOAT";
    } 
    //command = command.substr(0, command.length()-1);
    command += ");";
    return Execute(command.c_str()); //Create Table to store variable types
}
int DatabaseHandler::Log(std::string datetime, float measurements[]){
    std::string command = "INSERT INTO data ('datetime'";
    for(int i = 0; i < count; i++){
        command += ",'" + varlist[i] + "'";
    } 
    //command = command.substr(0, command.length()-1);
    command += ") VALUES ('" + datetime + "'";
    for(int i = 0; i < count; i++){
        command += ",'" + std::to_string(measurements[i]) + "'";
    }
    command += ")";
    return Execute(command.c_str()); //Store data
}
int DatabaseHandler::Execute(const char* command){ //Fast execute
    char *errorMSG = 0;
    int rc = sqlite3_exec(DatabaseHandler::database, command, DatabaseHandler::callback, (void*)DatabaseHandler::data, &errorMSG); //Execute command
    if(rc != SQLITE_OK){Serial.print("Failed executing command: ");Serial.println(errorMSG);return - 1;} //Checks for any errors
    return 0;
}
int DatabaseHandler::callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}
std::string DatabaseHandler::SelectAll(){
    std::string exec_string = "SELECT * FROM data";
    int rc = sqlite3_prepare_v2(DatabaseHandler::database, exec_string.c_str(), 1000, &DatabaseHandler::resource, &DatabaseHandler::tail);
    if(rc != SQLITE_OK){Serial.println("Failed executing command");return "";}
    while (sqlite3_step(DatabaseHandler::resource) == SQLITE_ROW)
    {
        std::string response = "";
        response += ((const char *)sqlite3_column_text(DatabaseHandler::resource, 0));
        response += "; ";
        for(int i = 1; i <= count; i++){
            response += ((const char *)sqlite3_column_text(DatabaseHandler::resource, i));
            response += "; ";
        } 
        Serial.println(response.c_str());
    }
    sqlite3_finalize(DatabaseHandler::resource);
    return "";
}
std::string DatabaseHandler::Select(std::string date_from, std::string date_to){
    std::string exec_string = "SELECT * FROM data WHERE datetime BETWEEN '" + date_from + ":00' AND '" + date_to + ":00'";
    int rc = sqlite3_prepare_v2(DatabaseHandler::database, exec_string.c_str(), 1000, &DatabaseHandler::resource, &DatabaseHandler::tail);
    if(rc != SQLITE_OK){Serial.println("Failed executing command");return "";}
    std::string response = "";
    std::string datetimes = "";
    std::string _data[count];
    for(int i = 0; i < count; i++){
        _data[i] = varlist[i] + ":";
    }
    while (sqlite3_step(DatabaseHandler::resource) == SQLITE_ROW)
    {
        datetimes += ((const char *)sqlite3_column_text(DatabaseHandler::resource, 0)); //Datetime
        datetimes += ",";
        for(int i = 1; i <= count; i++){
            _data[i-1] += ((const char *)sqlite3_column_text(DatabaseHandler::resource, i));
            _data[i-1] += ",";
        }
    }
    response = datetimes.substr(0, datetimes.length()-1) + "/";
    for(int i = 0; i < count; i++){
        response += _data[i].substr(0, _data[i].length()-1) + ";";
    }
    sqlite3_finalize(DatabaseHandler::resource);
    return response.substr(0, response.length()-1);
}
std::string DatabaseHandler::Select(std::string date_from, std::string date_to, int max){
    int nth = (int)(DatabaseHandler::GetCount(date_from, date_to) / max);
    nth = nth < 1 ? 1 : nth;
    std::string exec_string = "SELECT * FROM data WHERE datetime BETWEEN '" + date_from + ":00' AND '" + date_to + ":00' AND rowid % " + nth + " = 0";
    int rc = sqlite3_prepare_v2(DatabaseHandler::database, exec_string.c_str(), 1000, &DatabaseHandler::resource, &DatabaseHandler::tail);
    if(rc != SQLITE_OK){Serial.println("Failed executing command");return "";}
    std::string response = "";
    std::string datetimes = "";
    std::string _data[count];
    for(int i = 0; i < count; i++){
        _data[i] = varlist[i] + ":";
    }
    while (sqlite3_step(DatabaseHandler::resource) == SQLITE_ROW)
    {
        datetimes += ((const char *)sqlite3_column_text(DatabaseHandler::resource, 0)); //Datetime
        datetimes += ",";
        for(int i = 1; i <= count; i++){
            _data[i-1] += ((const char *)sqlite3_column_text(DatabaseHandler::resource, i));
            _data[i-1] += ",";
        }
    }
    response = datetimes.substr(0, datetimes.length()-1) + "/";
    for(int i = 0; i < count; i++){
        response += _data[i].substr(0, _data[i].length()-1) + ";";
    }
    sqlite3_finalize(DatabaseHandler::resource);
    return response.substr(0, response.length()-1);
}
int DatabaseHandler::GetCount(std::string date_from, std::string date_to){ //SELECT COUNT(*) from data
    std::string exec_string = "SELECT COUNT(*) FROM data WHERE datetime BETWEEN '" + date_from + ":00' AND '" + date_to + ":00'";
    int rc = sqlite3_prepare_v2(DatabaseHandler::database, exec_string.c_str(), 1000, &DatabaseHandler::resource, &DatabaseHandler::tail);
    if(rc != SQLITE_OK){Serial.println("Failed executing command");return "";}
    std::string response = "";
    while (sqlite3_step(DatabaseHandler::resource) == SQLITE_ROW)
    {
        response = ((const char *)sqlite3_column_text(DatabaseHandler::resource, 0));
    }
    return std::stoi(response);
}