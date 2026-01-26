//
// Created by churunfa on 2026/1/23.
//

#ifndef SWITCH_AUTO_CORE_DATABASEMANAGER_H
#define SWITCH_AUTO_CORE_DATABASEMANAGER_H

#include <iostream>
#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "base/BaseOperate.h"
#include "base/IdGenerate.h"
#include "combination/Combination.h"

using namespace sqlite_orm;

inline  auto createStorage(const std::string& path) {
    return make_storage(path,
        BaseOperate::getDescription(),
        Combination::getDescription(),
        CombinationNode::getDescription(),
        CombinationEdge::getDescription(),
        IdGenerate::getDescription()
    );
}

using Storage = decltype(createStorage(""));

class DatabaseManager {

    DatabaseManager() {
        try {
            const auto _storage = createNewConnection();
            BaseOperate::initData(*_storage);
            std::cout << "数据库已初始化并完成同步。" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "DATABASE FATAL ERROR: " << e.what() << std::endl;
            // 如果数据库都打不开，通常建议终止程序，避免后续空指针
            throw;
        }
    }
public:
    static std::unique_ptr<Storage> createNewConnection() {
        try {
            auto storage = std::make_unique<Storage>(createStorage(DB_FILE_PATH));
            // 只有主线程或者初始化线程需要 sync_schema，
            // 但 sqlite_orm 的 sync_schema 会安全处理“已存在”的情况
            storage->sync_schema();
            return storage;
        } catch (const std::exception& e) {
            std::cerr << "DATABASE CONNECTION ERROR: " << e.what() << std::endl;
            throw;
        }
    }

static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    // 禁止拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

inline Storage& get_db() {
    thread_local std::unique_ptr<Storage> local_storage = DatabaseManager::createNewConnection();
    return *local_storage;
}

#define db get_db()

#endif //SWITCH_AUTO_CORE_DATABASEMANAGER_H