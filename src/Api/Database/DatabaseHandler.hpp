#pragma once
#include <Arduino.h>
#include <string>
#include <sqlite3.h>
#include <SPIFFS.h>

class DatabaseHandler{
public:
    static int Init(std::string variables[], std::string units[], std::string colors[], int count);
    static int CheckDatabase();
    static int Log(std::string datetime, float measurements[]);
    static std::string Select(std::string date_from, std::string date_to);
    static std::string Select(std::string date_from, std::string date_to, int max);
    static int GetCount(std::string date_from, std::string date_to);
    static int Execute(const char* command);
    static std::string SelectAll();
private:
    static int count;
    static std::string *varlist;
    static int SetupDatabase();
    static sqlite3 *database;
    static sqlite3_stmt *resource;
    static const char *tail;
    static int callback(void *data, int argc, char **argv, char **azColName);
    static const char* data;
};