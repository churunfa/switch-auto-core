//
// Created by churunfa on 2026/1/23.
//

#ifndef SWITCH_AUTO_CORE_DATABASEMANAGER_H
#define SWITCH_AUTO_CORE_DATABASEMANAGER_H

#include <iostream>
#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "base/BaseOperate.h"
#include "combination/Combination.h"

using namespace sqlite_orm;

inline auto createStorage(const std::string& path) {
    return make_storage(path,
        BaseOperate::getDescription(),
        Combination::getDescription(),
        CombinationNode::getDescription(),
        CombinationEdge::getDescription()
    );
}

using Storage = decltype(createStorage(""));

class DatabaseManager {
    std::unique_ptr<Storage> _storage;

    DatabaseManager() {
        try {
            _storage = std::make_unique<Storage>(createStorage(DB_FILE_PATH));
            _storage->sync_schema();
            BaseOperate::initData(*_storage);
            std::cout << "数据库已初始化并完成同步。" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "DATABASE FATAL ERROR: " << e.what() << std::endl;
            // 如果数据库都打不开，通常建议终止程序，避免后续空指针
            throw;
        }
    }

public:
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    [[nodiscard]] Storage& getStorage() const { return *_storage; }

    // 禁止拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

inline auto& db = DatabaseManager::getInstance().getStorage();
extern Storage& db;

#endif //SWITCH_AUTO_CORE_DATABASEMANAGER_H