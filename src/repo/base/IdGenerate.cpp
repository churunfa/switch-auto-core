//
// Created by churunfa on 2026/1/26.
//
#include "IdGenerate.h"

#include "repo/DatabaseManager.h"

int IdGenerateRepo::generateId(const std::string& type) {
    std::lock_guard lock(generateIdMtx);
    try {
        const auto record = db.get_pointer<IdGenerate>(
            where(c(&IdGenerate::type) == type)
        );

        int newId = 1;
        if (record) {
            newId = record->id + 1;
        }
        db.replace(IdGenerate{ type, newId });
        return newId;
    } catch (const std::exception& e) {
        std::cerr << "Transaction failed: " << e.what() << std::endl;
        throw std::out_of_range("id生成失败");
    }
}