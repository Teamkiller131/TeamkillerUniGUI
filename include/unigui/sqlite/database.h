#pragma once
#include <functional>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <variant>
#include <vector>

namespace unigui::sqlite {

using Param = std::variant<int, double, std::string, std::nullptr_t, const char*>;

struct Row {
    std::vector<std::string> columns;
    std::string Get(int i) const { return i < (int) columns.size() ? columns[i] : ""; }
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
    int Execute(const std::string& sql, const std::vector<Param>& params = {});

    /// Query SELECT. Calls callback for each row. Returns row count.
    int Query(const std::string& sql, const std::vector<Param>& params,
              std::function<void(Row&)> callback);

    /// Get a single value (first column of first row).
    std::string QueryValue(const std::string& sql, const std::vector<Param>& params = {});

    /// Schema migration: run SQL if version not yet applied.
    bool Migrate(int version, const std::string& sql);

    /// Last insert row ID.
    int64_t LastInsertId();

    sqlite3* Raw() { return db_; }

private:
    sqlite3* db_ = nullptr;
    void BindParams(sqlite3_stmt* stmt, const std::vector<Param>& params);
};

class Transaction {
public:
    Transaction(Database& db)
            : db_(db) {
        db_.Execute("BEGIN");
    }
    ~Transaction() {
        if (!committed_)
            db_.Execute("ROLLBACK");
    }
    void Commit() {
        db_.Execute("COMMIT");
        committed_ = true;
    }

private:
    Database& db_;
    bool committed_ = false;
};

} // namespace unigui::sqlite
