//
// Created by churunfa on 2026/2/16.
//

#include "ButtonBinding.h"

#include "repo/DatabaseManager.h"
struct ButtonBinding;

std::vector<ButtonBinding> ButtonBindingRepo::allButtonBinding() {
    return db.get_all<ButtonBinding>();;
}
