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
    
    // Основные операции
    bool AddFile(const std::string& name, long long size, int catId, int ownerId);
    
    // Группа А: Продвинутый SQL и Логика
    void SearchByName(const std::string& partName);
    void GetFiles(const std::string& orderBy = "Name");
    void ShowStatistics();
    bool SoftDelete(int resourceId);
    bool RestoreFile(int resourceId);
    void ShowRecycleBin();

    // Группа Б: Валидация и Очистка
    void CleanupOldData();

    // Группа В: Интерфейс и UX
    void GetFilesPaged(int pageNum, int pageSize = 10);
    void ExportToCSV(const std::string& filename);
    int GetMaxNameLength();
    
    // Группа Д: Отчетность и поиск
    void ExportReport(const std::string& filename);
    void IntelligentSearch(const std::string& query);

    // Целостность
    bool DeleteCategory(int catId);
};
