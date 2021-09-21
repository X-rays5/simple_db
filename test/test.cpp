#include <cstdio>
#include <string>
#include <filesystem>
#include "../db.hpp"

int main() {
    if (std::filesystem::exists("testdb/test.simpledb")) {
        std::filesystem::remove("testdb/test.simpledb");
        printf("Deleted old db folder\n");
    }

    simple_db::DB db("testdb\\test.simpledb");

    {
        auto table = db["imatable"];
        if (table.Ok()) { // check if db ptr is valid
            assert(table.Put("hi there", "yes sir").ok());
        }
        printf("Put success\n");
    }

    {
        auto table = db["imatable"];
        if (table.Ok()) { // check if db ptr is valid
            auto res = table.Get("hi there");
            assert(res.status.ok() && res.value == "yes sir");
        }
        printf("Get success\n");
    }

    {
        auto table = db["imatable"];
        if (table.Ok()) { // check if db ptr is valid
            assert(table.Delete("hi there").ok());
        }
        if (table.Ok()) { // check if db ptr is valid
            auto res = table.Get("hi there");
            assert(!res.status.ok());
        }
        printf("Delete success\n");
    }

    {
        auto tables = db.GetTables();
        assert(tables.size() == 1);
        auto entry = tables.find("imatable");
        assert(entry != tables.end());
        printf("GetTables success\n");
    }
    return 0;
}
