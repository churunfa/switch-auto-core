//
// Created by churunfa on 2026/1/26.
//

#ifndef SWITCH_AUTO_CORE_IDGENERATE_H
#define SWITCH_AUTO_CORE_IDGENERATE_H
#include <string>
#include <mutex>
#include "sqlite_orm/sqlite_orm.h"

using namespace sqlite_orm;

struct IdGenerate {
    std::string type;
    int id;

    static auto getDescription() {
        return make_table("id_generate",
            make_column("type", &IdGenerate::type, primary_key()),
            make_column("id", &IdGenerate::id)
        );
    }
};

class IdGenerateRepo {
    static inline std::recursive_mutex generateIdMtx;
public:
    static int generateId(const std::string& type);
};

#endif //SWITCH_AUTO_CORE_IDGENERATE_H