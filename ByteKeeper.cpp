#include "DBManager.h"
#include <iostream>

using namespace std;

DBManager::DBManager() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC) {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
        cerr << "[Ошибка] Не удалось выделить дискриптор окружения." << endl;
    }
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
        cerr << "[Ошибка] Не удалось установить версию ODBC." << endl;
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
        cerr << "[Ошибка] Не удалось выделить дискриптор подключения." << endl;
        return false;
    }

    // Формируем строку подключения для SQLEXPRESS
    string connStr = "DRIVER={SQL Server};SERVER=DESKTOP-SR5B112\\SQLEXPRESS;DATABASE=" + databaseName + ";Trusted_Connection=yes;";
    
    SQLWCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    // Конвертация string в wstring для SQLDriverConnectW
    wstring wConnStr(connStr.begin(), connStr.end());

    SQLRETURN ret = SQLDriverConnectW(hDbc, NULL, (SQLWCHAR*)wConnStr.c_str(), SQL_NTS, outConnStr, 1024, &outConnStrLen, SQL_DRIVER_NOPROMPT);

    if (SQL_SUCCEEDED(ret)) {
        cout << "[DB] Успешное подключение к базе: " << databaseName << endl;
        return true;
    }
    else {
        cout << "[DB] Ошибка подключения к базе " << databaseName << endl;
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
    if (hDbc == SQL_NULL_HDBC) return false;

    SQLWCHAR state[6];
    SQLWCHAR msg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native;
    SQLSMALLINT msgLen;

    // Пытаемся получить диагностическую запись, чтобы проверить живо ли соединение
    SQLRETURN ret = SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, state, &native, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen);

    if (ret == SQL_NO_DATA) {
        cout << "[PING] Соединение активно." << endl;
        return true;
    }
    else {
        wcout << L"[PING] Проблема с соединением. Состояние: " << state << endl;
        return false;
    }
}

void DBManager::PrintError(SQLSMALLINT handleType, SQLHANDLE handle) {
    SQLWCHAR state[6], msg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native;
    SQLSMALLINT msgLen;
    int i = 1;

    while (SQLGetDiagRecW(handleType, handle, i++, state, &native, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen) != SQL_NO_DATA) {
        wcout << L"SQL Error: " << state << L" : " << msg << endl;
    }
}
