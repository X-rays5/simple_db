//
// Created by X-ray on 9/21/2021.
//

#pragma once

#ifndef SIMPLE_DB__DB_HPP_
#define SIMPLE_DB__DB_HPP_
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <inireader.hpp>
#include <leveldb/db.h>

namespace simple_db {
  // db closes when it goes out of scope
  struct TableSession {
    explicit TableSession(leveldb::DB* db) : db_(db)
    {
    }

    ~TableSession() {
      delete db_;
    }

    bool Ok(){
      return db_ != nullptr;
    }

    leveldb::Status Put(const std::string& key, const std::string& value) {
      return db_->Put(woptions_, key, value);
    }

    struct GetReturn {
      leveldb::Status status;
      std::string value;
    };

    GetReturn Get(const std::string& key) {
      std::string value;
      auto status = db_->Get(roptions_, key, &value);
      return {status, value};
    }

    leveldb::Status Delete(std::string key) {
      return db_->Delete(woptions_, key);
    }

    leveldb::WriteOptions& GetWriteOptions() {
      return woptions_;
    }

    leveldb::ReadOptions& GetReadOptions() {
      return roptions_;
    }
    
    // need to call delete iter;
    leveldb::Iterator* GetIterator() {
    	return db_->NewIterator(roptions_);
    }
   private:
    leveldb::DB* db_;
    leveldb::WriteOptions woptions_;
    leveldb::ReadOptions roptions_;
  };

  class DB {
   public:
    explicit DB(const std::string& dbname) : dbname_(dbname) {
      std::size_t pos = 0;
#ifdef WIN32
      if ((pos = dbname.rfind('\\')) != std::string::npos) {
#else
      if ((pos = dbname.rfind('/')) != std::string::npos) {
#endif
        dbdir_ = dbname.substr(0, pos + 1);
        if (!std::filesystem::is_directory(dbdir_))
            std::filesystem::create_directories(dbdir_);
      }

      if (!std::filesystem::exists(dbname)) {
        std::ofstream create_file(dbname);
        assert(create_file.is_open());
        create_file.close();
      }

      ini_.Parse(dbname);

      options_.create_if_missing = true;
    }

    ~DB() {
          std::ofstream writer(dbname_);
          assert(writer.is_open());
          writer << ini_.stringify();
          writer.close();
      }

    ini::Parser::Entries_t GetTables() {
      return ini_["tables"].entries;
    }

    leveldb::Status RepairTable(const std::string& table_name) {
        return leveldb::RepairDB(table_name, options_);
    }

    leveldb::Status DeleteTable(const std::string& table_name) {
      ini_.RemoveKV("tables", table_name);
      return leveldb::DestroyDB(table_name, options_);
    }

    // if table doesn't exist it wil be created
    TableSession operator[](const std::string& table_name) {
      leveldb::DB* db;
      ini_.AddKV("tables", table_name, dbdir_ + table_name);

      auto status = leveldb::DB::Open(options_, dbdir_ + table_name, &db);
      if (!status.ok()) {
        ini_.AddKV("errors", "open_" + table_name, status.ToString());
        return TableSession(nullptr);
      }

      return TableSession(db);
    }
   private:
      std::string dbname_;
      std::string dbdir_;
      ini::Parser ini_;
      leveldb::Options options_;
  };
} // simple_db
#endif //SIMPLE_DB__DB_HPP_
