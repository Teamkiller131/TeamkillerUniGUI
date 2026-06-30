#pragma once
#include <unigui/core/error.h>

#include <functional>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <variant>
#include <vector>

namespace unigui::sqlite {

using Param = std::variant<int, double, std::string, std::nullptr_t, const char*>;

struct Row {
    std::vector<std::string> columns; ///< Column values for this row.
    std::vector<std::string> names;   ///< Column names, parallel to `columns`.
    std::string Get(int i) const { return i < (int) columns.size() ? columns[i] : ""; }
    /// Value of the column named `name`, or "" if there is no such column.
    std::string Get(const char* name) const;
};

class Database {
public:
    Database() = default;
    ~Database();

    // Owns a raw sqlite3* (closed in the destructor), so copying would double-close the
    // same handle (double free / use-after-free) and a moved-from copy would close a
    // handle it no longer owns. Forbid copy; provide a handle-transferring move so the
    // common factory-return / container-store patterns still work with single ownership.
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept
            : db_(other.db_) {
        other.db_ = nullptr;
    }
    Database& operator=(Database&& other) noexcept {
        if (this != &other) {
            Close(); // release any handle we currently own (null-safe + idempotent)
            db_ = other.db_;
            other.db_ = nullptr;
        }
        return *this;
    }

    /// Open (or create) the database at `path`. Returns a value result on success,
    /// or `Err(ErrorCode::OpenFailed)` if sqlite could not open it (the detailed
    /// sqlite message is logged).
    Result<void> Open(const std::string& path);
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
    explicit Transaction(Database& db)
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
