#include <unigui/sqlite/database.h>

#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
using namespace unigui::sqlite;

class DBTest : public ::testing::Test {
protected:
    void SetUp() override { ASSERT_TRUE(db.Open(":memory:").has_value()); }
    void TearDown() override { db.Close(); }
    Database db;
};

// Open now returns Result<void> (4.0): a value result on success, OpenFailed on a
// path that sqlite cannot open.
TEST(DBOpen, Success_HasValue) {
    Database d;
    EXPECT_TRUE(d.Open(":memory:").has_value());
}
TEST(DBOpen, BadPath_ReturnsOpenFailed) {
    Database d;
    const auto r = d.Open("/no_such_dir_unigui/nested/does_not_exist.db");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), unigui::ErrorCode::OpenFailed);
}

TEST_F(DBTest, CreateTable_Execute) {
    db.Execute("CREATE TABLE t (id INT, name TEXT)");
    db.Execute("INSERT INTO t VALUES(?,?)", {1, std::string("Alice")});
    auto name = db.QueryValue("SELECT name FROM t WHERE id=1");
    EXPECT_EQ(name, "Alice");
}
TEST_F(DBTest, Transaction_Rollback) {
    db.Execute("CREATE TABLE t (x INT)");
    {
        Transaction txn(db);
        db.Execute("INSERT INTO t VALUES(1)");
    }
    int count = db.Query("SELECT * FROM t", {}, [](auto&) {});
    EXPECT_EQ(count, 0); // rolled back
}
TEST_F(DBTest, Migrate_AppliesSchema) {
    db.Migrate(1, "CREATE TABLE users (id INT PRIMARY KEY, name TEXT)");
    db.Migrate(1, "CREATE TABLE users (id INT)"); // idempotent
    db.Execute("INSERT INTO users VALUES(1,?)", {std::string("Bob")});
    EXPECT_EQ(db.QueryValue("SELECT name FROM users WHERE id=1"), "Bob");
}

// ── Error-path: Migrate with empty QueryValue must not crash ────────────────
TEST_F(DBTest, Migrate_EmptyQueryValue_DoesNotCrash) {
    // Fresh DB: MAX(version) returns 0 via COALESCE, so stoi would get "0".
    // Verify this works without exception.
    EXPECT_NO_THROW(db.Migrate(1, "CREATE TABLE t1 (id INT)"));
    EXPECT_TRUE(db.Migrate(1, "CREATE TABLE t1 (id INT)")); // idempotent
}

// ── Move semantics: Database owns a raw sqlite3*, so copy is deleted and move must
//    transfer the handle, leaving the source closed (no double-close on destruction). ──
TEST(DBMove, MoveConstruct_TransfersHandle) {
    Database a;
    ASSERT_TRUE(a.Open(":memory:").has_value());
    Database b(std::move(a));
    EXPECT_TRUE(b.IsOpen());
    EXPECT_FALSE(a.IsOpen()); // moved-from handle was nulled, won't be double-closed
    EXPECT_EQ(b.Execute("CREATE TABLE t (x INT)"), 0);
}
TEST(DBMove, MoveAssign_ClosesOwnHandleThenTransfers) {
    Database a, b;
    ASSERT_TRUE(a.Open(":memory:").has_value());
    ASSERT_TRUE(b.Open(":memory:").has_value());
    b = std::move(a); // b's own handle is closed, then a's is transferred in
    EXPECT_TRUE(b.IsOpen());
    EXPECT_FALSE(a.IsOpen());
}
static_assert(!std::is_copy_constructible_v<Database>, "Database copy must be deleted");
static_assert(!std::is_copy_assignable_v<Database>, "Database copy-assign must be deleted");
static_assert(std::is_move_constructible_v<Database>, "Database must be movable");

// ── Row::Get(const char*): name-based column access (was declared but never
//    defined — a guaranteed linker error for any caller). This test references the
//    symbol so a future missing definition is caught at link time. ──
TEST_F(DBTest, Row_GetByColumnName) {
    db.Execute("CREATE TABLE t (id INT, name TEXT)");
    db.Execute("INSERT INTO t VALUES(?,?)", {7, std::string("Carol")});
    std::string gotId, gotName, gotMissing = "sentinel";
    int n = db.Query("SELECT id, name FROM t", {}, [&](Row& r) {
        gotId = r.Get("id");
        gotName = r.Get("name");
        gotMissing = r.Get("nope"); // unknown column -> ""
    });
    EXPECT_EQ(n, 1);
    EXPECT_EQ(gotId, "7");
    EXPECT_EQ(gotName, "Carol");
    EXPECT_EQ(gotMissing, "");
}
