#include "DBManager.h"
#include <iostream>

using namespace std;

DBManager::DBManager() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC) {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
        cerr << "[Ш] шибка выделения дескриптора окружения (ENV)." << endl;
    }
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
        cerr << "[Ш] шибка установки версии ODBC." << endl;
    }
}

DBManager::~DBManager() {
    Disconnect();
    if (hEnv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

bool DBManager::Connect(const string& databaseName) {
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
        cerr << "[Ш] шибка выделения дескриптора DBC." << endl;
        return false;
    }

    string connStr = "DRIVER={SQL Server};SERVER=DESKTOP-SR5B112\\SQLEXPRESS;DATABASE=" + databaseName + ";Trusted_Connection=yes;";
    wstring wConnStr(connStr.begin(), connStr.end());
    SQLWCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    SQLRETURN ret = SQLDriverConnectW(hDbc, NULL, (SQLWCHAR*)wConnStr.c_str(), SQL_NTS, outConnStr, 1024, &outConnStrLen, SQL_DRIVER_NOPROMPT);

    if (SQL_SUCCEEDED(ret)) {
        cout << "[DB] одключено к " << databaseName << endl;
        return true;
    } else {
        cerr << "[DB] шибка подключения к " << databaseName << endl;
        PrintError(SQL_HANDLE_DBC, hDbc);
        return false;
    }
}

void DBManager::Disconnect() {
    if (hDbc != SQL_NULL_HDBC) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        hDbc = SQL_NULL_HDBC;
    }
}

bool DBManager::ChangeDatabase(const string& newDatabaseName) {
    Disconnect();
    return Connect(newDatabaseName);
}

bool DBManager::Ping() {
    if (hDbc == SQL_NULL_HDBC) {
        cout << "[PING] Соединение отсутствует." << endl;
        return false;
    }

    SQLWCHAR serverName[128], serverVer[128];
    SQLSMALLINT len;

    // асширенная информация через SQLGetInfoW (Commit 14)
    SQLGetInfoW(hDbc, SQL_DBMS_NAME, serverName, 128, &len);
    SQLGetInfoW(hDbc, SQL_DBMS_VER, serverVer, 128, &len);

    wcout << L"[PING] Сервер: " << serverName << L" (ерсия: " << serverVer << L")" << endl;
    cout << "[PING] Соединение активно." << endl;
    return true;
}

void DBManager::PrintError(SQLSMALLINT handleType, SQLHANDLE handle) {
    SQLWCHAR state[6], msg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native;
    SQLSMALLINT msgLen;
    int i = 1;
    while (SQLGetDiagRecW(handleType, handle, i++, state, &native, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen) != SQL_NO_DATA) {
        wcout << L"SQL-ошибка: " << state << L" : " << msg << endl;
    }
}
