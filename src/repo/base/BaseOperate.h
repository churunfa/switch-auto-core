//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_BASE_H
#define SWITCH_AUTO_CORE_BASE_H

#include <sqlite_orm/sqlite_orm.h>

using namespace sqlite_orm;

struct BaseOperate {
    int id;
    std::string ename;
    std::string name;
    int param_size;
    std::string param_names;
    std::string init_params;
    int min_exec_time;
    int min_reset_time;
    bool need_reset = true;

    static auto getDescription() {
        return make_table("base_operate",
            make_column("id", &BaseOperate::id, primary_key().autoincrement()),
            make_column("ename", &BaseOperate::ename, unique()),
            make_column("name", &BaseOperate::name, unique()),
            make_column("param_size", &BaseOperate::param_size),
            make_column("param_names", &BaseOperate::param_names),
            make_column("init_names", &BaseOperate::init_params),
            make_column("min_exec_time", &BaseOperate::min_exec_time),
            make_column("min_reset_time", &BaseOperate::min_reset_time),
            make_column("need_reset", &BaseOperate::need_reset)
        );
    }

    static const std::vector<BaseOperate>& getStaticDefaults() {
        static int default_min_time = 15;
        static const std::vector<BaseOperate> defaults = {
            {0, "BUTTON_Y", "Y", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_X", "X", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_B", "B", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_A", "A", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_R", "R", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_ZR", "ZR", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_MINUS", "-", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_PLUS", "+", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_THUMB_R", "R按下", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_THUMB_L", "L按下", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_HOME", "HOME", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_CAPTURE", "📷", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "DPAD_DOWN", "⬇️", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "DPAD_UP", "⬆️", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "DPAD_RIGHT", "➡️", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "DPAD_LEFT", "⬅️", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_L", "L", 0, "[]", "[]", default_min_time, default_min_time},
            {0, "BUTTON_ZL", "ZL", 0, "[]", "[]", default_min_time, default_min_time},

            {0, "LEFT_STICK", "左遥感", 2, R"(["x","y"])", "[0, 0]", default_min_time, default_min_time},
            {0, "RIGHT_STICK", "右遥感", 2, R"(["x","y"])", "[0, 0]", default_min_time, default_min_time},

            {0, "IMU", "体感", 6, R"(["accX","accY","accZ","gyroX","gyroY","gyroZ"])", "[0, 0, 4096, 0, 0, 0]", default_min_time, default_min_time},

            {0, "LEFT_STICK_CIRCLE", "左遥感旋转一圈", 0, "[]", "[]", default_min_time * 8, 0, false},
            {0, "RESET_ALL", "重置所有按键", 0, "[]", "[]", default_min_time, default_min_time, false},
            {0, "SLEEP", "等待时间", 0, "[]", "[]", 200, 0, false},
            {0, "START_EMPTY", "开始", 0, "[]", "[]", 0, 0, false},
        };
        return defaults;
    }

    template<typename T>
    static void initData(T& db){
        db.transaction([&]() -> bool {
            db.template remove_all<BaseOperate>();
            int idx = 0;
            for (auto operate : getStaticDefaults()) {
                operate.id = ++idx;
                db.template replace<BaseOperate>(operate);
            }
            return true;
        });
    }
};

class BaseOperateRepo {
public:
    static std::vector<BaseOperate> findAll();
    static std::optional<BaseOperate> findOneById(int id);
    static std::optional<BaseOperate> findOneByEname(std::string ename);
};

#endif //SWITCH_AUTO_CORE_BASE_H