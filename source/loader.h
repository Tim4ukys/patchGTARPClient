#pragma once

class loader : public std::vector<std::function<void()>> {

	using callBack_fnc = void();

	void add_before(std::function<callBack_fnc> fnc);
    void add_after(std::function<callBack_fnc> fnc); 
};
