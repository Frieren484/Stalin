#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

class DBManager {
private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    void PrintError(SQLSMALLINT handleType, SQLHANDLE handle);

public:
    DBManager();
    ~DBManager();

    bool Connect(const std::string& dbName);
    void Disconnect();
    bool Ping();
    bool ChangeDatabase(const std::string& newDb);
    SQLHDBC GetConnection() { return hDbc; }
};
