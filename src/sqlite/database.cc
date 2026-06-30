#include <unigui/core/log.h>
#include <unigui/sqlite/database.h>

#include <cstdlib>
#include <cstring>

namespace unigui::sqlite {

std::string Row::Get(const char* name) const {
    if (!name)
        return "";
    for (size_t i = 0; i < names.size(); i++)
        if (names[i] == name)
            return i < columns.size() ? columns[i] : "";
    return "";
}

Database::~Database() {
    Close();
}

Result<void> Database::Open(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        UNIGUI_LOG_ERROR("SQLite open failed: {} ({})", path, sqlite3_errmsg(db_));
        // sqlite3_open allocates a usable handle even on failure (so sqlite3_errmsg works,
        // read above); per the C API it must still be closed or the connection leaks.
        sqlite3_close(db_);
        db_ = nullptr;
        return Err(ErrorCode::OpenFailed);
    }
    // Enable WAL mode for concurrency
    Execute("PRAGMA journal_mode=WAL");
    UNIGUI_LOG_INFO("SQLite opened: {}", path);
    return {};
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
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        UNIGUI_LOG_ERROR("SQL step failed: {}", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return -1;
    }
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
    // Column names are identical for every row — resolve them once.
    std::vector<std::string> colNames;
    colNames.reserve(colCount);
    for (int i = 0; i < colCount; i++) {
        const char* n = sqlite3_column_name(stmt, i);
        colNames.emplace_back(n ? n : "");
    }
    int rows = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        row.names = colNames;
        row.columns.reserve(colCount);
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
    std::string verStr = QueryValue("SELECT COALESCE(MAX(version),0) FROM _migrations");
    int current = 0;
    if (!verStr.empty()) {
        char* end = nullptr;
        long v = std::strtol(verStr.c_str(), &end, 10);
        if (end != verStr.c_str())
            current = static_cast<int>(v);
    }
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
        // The DDL succeeded; record + commit it. If the bookkeeping INSERT or the COMMIT
        // fails (locked DB / busy / disk full), roll back and report failure rather than
        // claiming success for a migration that did not durably land.
        if (Execute("INSERT OR REPLACE INTO _migrations VALUES(?)", {version}) < 0) {
            UNIGUI_LOG_ERROR("Migration {} bookkeeping insert failed", version);
            Execute("ROLLBACK");
            return false;
        }
        if (Execute("COMMIT") < 0) {
            UNIGUI_LOG_ERROR("Migration {} commit failed", version);
            Execute("ROLLBACK");
            return false;
        }
        UNIGUI_LOG_INFO("Migration {} applied", version);
    }
    return true;
}

int64_t Database::LastInsertId() {
    return db_ ? sqlite3_last_insert_rowid(db_) : 0;
}

} // namespace unigui::sqlite
