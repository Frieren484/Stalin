#include "ResourceManager.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

ResourceManager::ResourceManager(SQLHDBC connection) : hDbc(connection) {}

void ResourceManager::LogAction(const string& description) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string query = "INSERT INTO Logs (ActionDate, ActionDescription) VALUES (GETDATE(), ?)";
    wstring wQuery(query.begin(), query.end());
    wstring wDesc(description.begin(), description.end());

    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wDesc.length(), 0, (SQLPOINTER)wDesc.c_str(), 0, NULL);

    SQLExecute(hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool ResourceManager::AddFile(const string& name, long long size, int catId, int ownerId) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string query = "INSERT INTO Resources (Name, Size, CategoryID, OwnerID, isDeleted, CreatedAt) VALUES (?, ?, ?, ?, 0, GETDATE())";
    wstring wQuery(query.begin(), query.end());
    wstring wName(name.begin(), name.end());

    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wName.length(), 0, (SQLPOINTER)wName.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &size, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &catId, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ownerId, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "[SUCCESS] Added file: " << name << endl;
        LogAction("Added file: " + name);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    } else {
        cerr << "[ERROR] Failed to add file." << endl;
        PrintError(SQL_HANDLE_STMT, hStmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }
}

void ResourceManager::SearchByName(const string& partName) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string query = "SELECT ResourceID, Name, Size FROM Resources WHERE Name LIKE ? AND isDeleted = 0";
    wstring wQuery(query.begin(), query.end());
    string pattern = "%" + partName + "%";
    wstring wPattern(pattern.begin(), pattern.end());

    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wPattern.length(), 0, (SQLPOINTER)wPattern.c_str(), 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(10) << "Size" << endl;
        cout << string(35, '-') << endl;

        SQLINTEGER id;
        SQLWCHAR name[256];
        SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            
            wstring wn(name);
            string n(wn.begin(), wn.end());
            if (n.length() > 15) n = n.substr(0, 12) + "...";

            cout << left << setw(5) << id << setw(20) << n << setw(10) << size << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::GetFiles(const string& orderBy) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string col = "Name";
    if (orderBy == "Size") col = "Size";
    else if (orderBy == "ID") col = "ResourceID";

    string query = "SELECT ResourceID, Name, Size FROM Resources WHERE isDeleted = 0 ORDER BY " + col;
    wstring wQuery(query.begin(), query.end());

    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS))) {
        cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(10) << "Size" << endl;
        cout << string(35, '-') << endl;

        SQLINTEGER id;
        SQLWCHAR name[256];
        SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            
            wstring wn(name);
            string n(wn.begin(), wn.end());
            if (n.length() > 15) n = n.substr(0, 12) + "...";

            cout << left << setw(5) << id << setw(20) << n << setw(10) << size << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::ShowStatistics() {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    SQLWCHAR* query = (SQLWCHAR*)L"SELECT COUNT(*), SUM(Size) FROM Resources WHERE isDeleted = 0";

    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLINTEGER count;
            SQLBIGINT totalSize;
            SQLLEN cbCount, cbSize;

            SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, &cbCount);
            SQLGetData(hStmt, 2, SQL_C_SBIGINT, &totalSize, 0, &cbSize);

            cout << "\n=== STATISTICS ===" << endl;
            cout << "Total files: " << count << endl;
            cout << "Total weight: " << (cbSize == SQL_NULL_DATA ? 0 : totalSize) << " bytes" << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool ResourceManager::SoftDelete(int resourceId) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string query = "UPDATE Resources SET isDeleted = 1 WHERE ResourceID = ?";
    wstring wQuery(query.begin(), query.end());

    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &resourceId, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "[SUCCESS] Moved to Trash (ID: " << resourceId << ")" << endl;
        LogAction("Moved file to trash, ID: " + to_string(resourceId));
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}

bool ResourceManager::RestoreFile(int resourceId) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    string query = "UPDATE Resources SET isDeleted = 0 WHERE ResourceID = ?";
    wstring wQuery(query.begin(), query.end());

    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &resourceId, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "[SUCCESS] Restored file (ID: " << resourceId << ")" << endl;
        LogAction("Restored file from trash, ID: " + to_string(resourceId));
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}

void ResourceManager::ShowRecycleBin() {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    SQLWCHAR* query = (SQLWCHAR*)L"SELECT ResourceID, Name FROM Resources WHERE isDeleted = 1";

    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        cout << "\n=== RECYCLE BIN ===" << endl;
        cout << left << setw(5) << "ID" << setw(20) << "Name" << endl;
        
        SQLINTEGER id;
        SQLWCHAR name[256];
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            
            wstring wn(name);
            string n(wn.begin(), wn.end());
            cout << left << setw(5) << id << setw(20) << n << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::PrintError(SQLSMALLINT handleType, SQLHANDLE handle) {
    SQLWCHAR state[6], msg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native;
    SQLSMALLINT msgLen;
    int i = 1;

    while (SQLGetDiagRecW(handleType, handle, i++, state, &native, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen) != SQL_NO_DATA) {
        wcout << L"SQL Error: " << state << L" : " << msg << endl;
    }
}
