#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <regex>

/**
 * ласс ResourceManager
 * твечает за логику работы с цифровыми активами в .
 */
class ResourceManager {
private:
    SQLHDBC hDbc;
    void LogAction(const std::string& description);
    void PrintError(SQLSMALLINT handleType, SQLHANDLE handle);
    bool IsDuplicate(const std::string& name);

public:
    ResourceManager(SQLHDBC connection);
    
    // сновные операции
    bool AddFile(const std::string& name, long long size, int catId, int ownerId);
    
    // руппа : родвинутый SQL
    void SearchByName(const std::string& partName);
    void GetFiles(const std::string& orderBy = "Name");
    void ShowStatistics();
    bool SoftDelete(int resourceId);
    bool RestoreFile(int resourceId);
    void ShowRecycleBin();

    // руппа : алидация
    void CleanupOldData();

    // руппа : нтерфейс и UX
    void GetFilesPaged(int pageNum, int pageSize = 10);
    void ExportToCSV(const std::string& filename);
    int GetMaxNameLength();
    
    // руппа : оиск и отчетность
    void ExportReport(const std::string& filename);
    void IntelligentSearch(const std::string& query);

    // руппа : елостность
    bool DeleteCategory(int catId);
};
