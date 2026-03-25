#include "ResourceManager.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <ctime>

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

bool ResourceManager::IsDuplicate(const string& name) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    string query = "SELECT COUNT(*) FROM Resources WHERE Name = ?";
    wstring wQuery(query.begin(), query.end());
    wstring wName(name.begin(), name.end());
    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wName.length(), 0, (SQLPOINTER)wName.c_str(), 0, NULL);
    SQLExecute(hStmt);
    SQLINTEGER count = 0;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return count > 0;
}

bool ResourceManager::AddFile(const string& name, long long size, int catId, int ownerId) {
    regex sysSymbols("[\\\\/:*?\"<>|]");
    if (regex_search(name, sysSymbols)) {
        cerr << "[Ш] мя содержит недопустимые символы!" << endl;
        return false;
    }
    vector<string> whitelist = {".exe", ".txt", ".pdf"};
    bool validExt = false;
    for (const auto& ext : whitelist) {
        if (name.length() >= ext.length() && name.compare(name.length() - ext.length(), ext.length(), ext) == 0) {
            validExt = true;
            break;
        }
    }
    if (!validExt) {
        cerr << "[Ш] асширение запрещено! опустимы: .exe, .txt, .pdf" << endl;
        return false;
    }
    if (IsDuplicate(name)) {
        cerr << "[Ш] айл с таким именем уже существует!" << endl;
        return false;
    }
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
        cout << "[СХ] айл добавлен: " << name << endl;
        LogAction("обавлен файл: " + name);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
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
        int w = GetMaxNameLength() + 5;
        if (w < 20) w = 20;
        cout << left << setw(5) << "ID" << setw(w) << "мя" << setw(10) << "азмер" << endl;
        cout << string(w + 15, '-') << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            if (n.length() > 15) n = n.substr(0, 12) + "...";
            cout << left << setw(5) << id << setw(w) << n << setw(10) << size << endl;
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
        int w = GetMaxNameLength() + 5;
        if (w < 20) w = 20;
        cout << left << setw(5) << "ID" << setw(w) << "мя" << setw(10) << "азмер" << endl;
        cout << string(w + 15, '-') << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            if (n.length() > 15) n = n.substr(0, 12) + "...";
            cout << left << setw(5) << id << setw(w) << n << setw(10) << size << endl;
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
            SQLINTEGER count; SQLBIGINT totalSize; SQLLEN cbSize;
            SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_SBIGINT, &totalSize, 0, &cbSize);
            cout << "\n=== СТТСТ ===" << endl;
            cout << "сего файлов: " << count << endl;
            cout << "бщий вес: " << (cbSize == SQL_NULL_DATA ? 0 : totalSize) << " байт" << endl;
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
        cout << "[СХ] айл перемещен в корзину (ID: " << resourceId << ")" << endl;
        LogAction("айл перемещен в корзину, ID: " + to_string(resourceId));
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
        cout << "[СХ] айл восстановлен (ID: " << resourceId << ")" << endl;
        LogAction("айл восстановлен из корзины, ID: " + to_string(resourceId));
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
        cout << "\n===  ===" << endl;
        cout << left << setw(5) << "ID" << setw(20) << "мя" << endl;
        SQLINTEGER id; SQLWCHAR name[256];
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            cout << left << setw(5) << id << setw(20) << n << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::CleanupOldData() {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    SQLWCHAR* query = (SQLWCHAR*)L"DELETE FROM Resources WHERE DATEDIFF(day, CreatedAt, GETDATE()) > 30 AND isDeleted = 1";
    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        SQLLEN count; SQLRowCount(hStmt, &count);
        cout << "[СТ] далено " << count << " старых файлов." << endl;
        LogAction("чистка: удалено " + to_string(count) + " старых файлов.");
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::GetFilesPaged(int pageNum, int pageSize) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    int offset = (pageNum - 1) * pageSize;
    string query = "SELECT ResourceID, Name, Size FROM Resources WHERE isDeleted = 0 ORDER BY ResourceID OFFSET ? ROWS FETCH NEXT ? ROWS ONLY";
    wstring wQuery(query.begin(), query.end());
    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &pageSize, 0, NULL);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &offset, 0, NULL);
    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "\n--- СТ " << pageNum << " ---" << endl;
        cout << left << setw(5) << "ID" << setw(20) << "мя" << setw(10) << "азмер" << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            cout << left << setw(5) << id << setw(20) << n << setw(10) << size << endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::ExportToCSV(const string& filename) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    SQLWCHAR* query = (SQLWCHAR*)L"SELECT ResourceID, Name, Size, CreatedAt FROM Resources";
    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        ofstream file(filename);
        file << "ID;мя;азмер;ата" << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size; SQLWCHAR date[64];
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            SQLGetData(hStmt, 4, SQL_C_WCHAR, date, sizeof(date), NULL);
            wstring wn(name); string sn(wn.begin(), wn.end());
            wstring wd(date); string sd(wd.begin(), wd.end());
            file << id << ";" << sn << ";" << size << ";" << sd << endl;
        }
        file.close();
        cout << "[СТ] CSV сохранен в " << filename << endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void ResourceManager::ExportReport(const string& filename) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    SQLWCHAR* query = (SQLWCHAR*)L"SELECT ResourceID, Name, Size FROM Resources WHERE isDeleted = 0";
    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        ofstream file(filename);
        time_t now = time(0);
        char* dt = ctime(&now);
        file << "=== ТТ  СС ByteKeeper ===" << endl;
        file << "ата формирования: " << dt << endl;
        file << left << setw(5) << "ID" << setw(30) << "мя ресурса" << setw(15) << "азмер (байт)" << endl;
        file << string(55, '=') << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            file << left << setw(5) << id << setw(30) << n << setw(15) << size << endl;
        }
        file.close();
        cout << "[ТТ] ро отчет сохранен в " << filename << endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

int ResourceManager::GetMaxNameLength() {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    SQLWCHAR* query = (SQLWCHAR*)L"SELECT MAX(LEN(Name)) FROM Resources";
    SQLINTEGER maxLen = 20;
    if (SQL_SUCCEEDED(SQLExecDirectW(hStmt, query, SQL_NTS))) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &maxLen, 0, NULL);
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (maxLen > 0) ? maxLen : 20;
}

void ResourceManager::IntelligentSearch(const string& query) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    stringstream ss(query);
    string word;
    string sqlQuery = "SELECT ResourceID, Name, Size FROM Resources WHERE isDeleted = 0 AND (";
    vector<string> words;
    while (ss >> word) {
        if (!words.empty()) sqlQuery += " OR ";
        sqlQuery += "Name LIKE ?";
        words.push_back("%" + word + "%");
    }
    sqlQuery += ")";
    if (words.empty()) return;
    wstring wQuery(sqlQuery.begin(), sqlQuery.end());
    SQLPrepareW(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);
    vector<wstring> wWords;
    for (size_t i = 0; i < words.size(); ++i) {
        wWords.push_back(wstring(words[i].begin(), words[i].end()));
        SQLBindParameter(hStmt, i + 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wWords[i].length(), 0, (SQLPOINTER)wWords[i].c_str(), 0, NULL);
    }
    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "\n--- ЬТТЫ С ---" << endl;
        SQLINTEGER id; SQLWCHAR name[256]; SQLBIGINT size;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &id, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 3, SQL_C_SBIGINT, &size, 0, NULL);
            wstring wn(name); string n(wn.begin(), wn.end());
            cout << "[ID: " << id << "] " << n << " (" << size << " байт)" << endl;
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
        wcout << L"SQL шибка: " << state << L" : " << msg << endl;
    }
}

bool ResourceManager::DeleteCategory(int catId) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    
    // роверка целостности: есть ли файлы в этой категории?
    string checkQuery = "SELECT COUNT(*) FROM Resources WHERE CategoryID = ? AND isDeleted = 0";
    wstring wCheck(checkQuery.begin(), checkQuery.end());
    SQLPrepareW(hStmt, (SQLWCHAR*)wCheck.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &catId, 0, NULL);
    SQLExecute(hStmt);
    SQLINTEGER count = 0;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
    }
    SQLFreeStmt(hStmt, SQL_CLOSE);

    if (count > 0) {
        cerr << "[] атегория содержит активные файлы! даление запрещено." << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    string delQuery = "DELETE FROM Categories WHERE CategoryID = ?";
    wstring wDel(delQuery.begin(), delQuery.end());
    SQLPrepareW(hStmt, (SQLWCHAR*)wDel.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &catId, 0, NULL);
    
    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << "[СХ] атегория удалена." << endl;
        LogAction("далена категория ID: " + to_string(catId));
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}
