#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <sqlite3.h>

namespace unigui::v2 {

using SqlParam = std::variant<int, double, std::string, std::nullptr_t, const char*>;

struct SqlRow {
    std::vector<std::string> columns;
    std::string Get(int i) const { return i<(int)columns.size()?columns[i]:""; }
    std::string Get(const char* name); // not implemented inline — needs column index
};

class Database {
public:
    Database() = default;
    ~Database();

    bool Open(const std::string& path);
    void Close();
    bool IsOpen() const { return db_ != nullptr; }

    /// Execute INSERT/UPDATE/DELETE. Returns affected rows or -1.
    int Execute(const std::string& sql, const std::vector<SqlParam>& params = {});

    /// Query SELECT. Calls callback for each row. Returns row count.
    int Query(const std::string& sql, const std::vector<SqlParam>& params,
              std::function<void(SqlRow&)> callback);

    /// Get a single value (first column of first row).
    std::string QueryValue(const std::string& sql, const std::vector<SqlParam>& params = {});

    /// Schema migration: run SQL if version not yet applied.
    bool Migrate(int version, const std::string& sql);

    /// Last insert row ID.
    int64_t LastInsertId();

    sqlite3* Raw() { return db_; }

private:
    sqlite3* db_ = nullptr;
    void BindParams(sqlite3_stmt* stmt, const std::vector<SqlParam>& params);
};

class Transaction {
public:
    Transaction(Database& db) : db_(db) { db_.Execute("BEGIN"); }
    ~Transaction() { if (!committed_) db_.Execute("ROLLBACK"); }
    void Commit() { db_.Execute("COMMIT"); committed_ = true; }
private:
    Database& db_; bool committed_ = false;
};

} // namespace unigui::v2
