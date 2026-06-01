#pragma once
#include "sqlite3.h"
#include <string>
#include <stdexcept>

class DatabaseManager {
public:
    DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();
    bool AddRecord(const std::wstring& col1, const std::wstring& col2);

private:
    sqlite3* m_db;

    void CreateTableIfNotExists();
};

