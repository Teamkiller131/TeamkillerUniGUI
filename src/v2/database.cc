#include <unigui/core/log.h>
#include <unigui/sqlite/database.h>

#include <cstring>

namespace unigui::sqlite {

Database::~Database() {
    Close();
}

bool Database::Open(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        UNIGUI_LOG_ERROR("SQLite open failed: {} ({})", path, sqlite3_errmsg(db_));
        db_ = nullptr;
        return false;
    }
    // Enable WAL mode for concurrency
    Execute("PRAGMA journal_mode=WAL");
    UNIGUI_LOG_INFO("SQLite opened: {}", path);
    return true;
}

void Database::Close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::BindParams(sqlite3_stmt* stmt, const std::vector<Param>& params) {
    for (int i = 0; i < (int) params.size(); i++) {
        int idx = i + 1;
        auto& p = params[i];
        if (std::holds_alternative<int>(p))
            sqlite3_bind_int(stmt, idx, std::get<int>(p));
        else if (std::holds_alternative<double>(p))
            sqlite3_bind_double(stmt, idx, std::get<double>(p));
        else if (std::holds_alternative<std::string>(p)) {
            auto& s = std::get<std::string>(p);
            sqlite3_bind_text(stmt, idx, s.c_str(), (int) s.size(), SQLITE_TRANSIENT);
        } else if (std::holds_alternative<const char*>(p))
            sqlite3_bind_text(stmt, idx, std::get<const char*>(p), -1, SQLITE_TRANSIENT);
        else
            sqlite3_bind_null(stmt, idx);
    }
}

int Database::Execute(const std::string& sql, const std::vector<Param>& params) {
    if (!db_)
        return -1;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        UNIGUI_LOG_ERROR("SQL prepare failed: {}", sqlite3_errmsg(db_));
        return -1;
    }
    BindParams(stmt, params);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes;
}

int Database::Query(const std::string& sql, const std::vector<Param>& params,
                    std::function<void(Row&)> callback) {
    if (!db_ || !callback)
        return -1;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        UNIGUI_LOG_ERROR("SQL query failed: {}", sqlite3_errmsg(db_));
        return -1;
    }
    BindParams(stmt, params);
    int colCount = sqlite3_column_count(stmt);
    int rows = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        for (int i = 0; i < colCount; i++) {
            const char* val = (const char*) sqlite3_column_text(stmt, i);
            row.columns.push_back(val ? val : "");
        }
        callback(row);
        rows++;
    }
    sqlite3_finalize(stmt);
    return rows;
}

std::string Database::QueryValue(const std::string& sql, const std::vector<Param>& params) {
    std::string result;
    Query(sql, params, [&](Row& r) {
        if (r.columns.size() > 0)
            result = r.columns[0];
    });
    return result;
}

bool Database::Migrate(int version, const std::string& sql) {
    if (!db_)
        return false;
    Execute("CREATE TABLE IF NOT EXISTS _migrations (version INT PRIMARY KEY)");
    int current = std::stoi(QueryValue("SELECT COALESCE(MAX(version),0) FROM _migrations"));
    if (version > current) {
        Execute("BEGIN");
        char* err = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
            UNIGUI_LOG_ERROR("Migration {} failed: {}", version, err ? err : "unknown");
            if (err)
                sqlite3_free(err);
            Execute("ROLLBACK");
            return false;
        }
        Execute("INSERT OR REPLACE INTO _migrations VALUES(?)", {version});
        Execute("COMMIT");
        UNIGUI_LOG_INFO("Migration {} applied", version);
    }
    return true;
}

int64_t Database::LastInsertId() {
    return db_ ? sqlite3_last_insert_rowid(db_) : 0;
}

} // namespace unigui::sqlite
