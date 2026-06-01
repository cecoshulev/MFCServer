#include "pch.h"
#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(const std::string& dbPath) : m_db(nullptr) {
    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " +
            std::string(sqlite3_errmsg(m_db)));
    }
    CreateTableIfNotExists();
}

DatabaseManager::~DatabaseManager() {
    if (m_db) sqlite3_close(m_db);
}

bool DatabaseManager::AddRecord(const std::wstring& col1, const std::wstring& col2)
{
    const char* sql =
        "INSERT INTO LogTable (time, mesg) "
        "VALUES (?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text16(stmt, 1, col1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text16(stmt, 2, col2.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

void DatabaseManager::CreateTableIfNotExists() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS LogTable ("
        "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  time TEXT NOT NULL,"
        "  mesg TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create table: " + err);
    }
}
