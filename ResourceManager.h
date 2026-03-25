#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <regex>

class ResourceManager {
private:
    SQLHDBC hDbc;
    void LogAction(const std::string& description);
    void PrintError(SQLSMALLINT handleType, SQLHANDLE handle);
    bool IsDuplicate(const std::string& name);

public:
    ResourceManager(SQLHDBC connection);
    
    // Core operations
    bool AddFile(const std::string& name, long long size, int catId, int ownerId);
    
    // Group A: Advanced SQL and Logic
    void SearchByName(const std::string& partName);
    void GetFiles(const std::string& orderBy = "Name");
    void ShowStatistics();
    bool SoftDelete(int resourceId);
    bool RestoreFile(int resourceId);
    void ShowRecycleBin();

    // Group B: Validations & Cleanup
    void CleanupOldData();
};
