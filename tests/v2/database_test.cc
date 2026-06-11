#include <unigui/sqlite/database.h>

#include <gtest/gtest.h>
using namespace unigui::sqlite;

class DBTest : public ::testing::Test {
protected:
    void SetUp() override { db.Open(":memory:"); }
    void TearDown() override { db.Close(); }
    Database db;
};

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
