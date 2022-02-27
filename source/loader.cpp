#include "pch.h"
#include "loader.h"

void loader::add_before(std::function<callBack_fnc> fnc) {
    insert(begin(), fnc);
}

void loader::add_after(std::function<callBack_fnc> fnc) {
    push_back(fnc);
}