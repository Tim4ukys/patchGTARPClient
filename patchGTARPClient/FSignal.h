/****************************************************
*                                                   *
*    Developer: Tim4ukys                            *
*                                                   *
*    email: tim4ukys.dev@yandex.ru                  *
*    vk: vk.com/tim4ukys                            *
*                                                   *
*    License: GNU GPLv3                             *
*                                                   *
****************************************************/
#pragma once
template<typename T>
class FSignal : private std::vector<std::function<T>> {
public:
    //FSignal();
    inline FSignal& operator+=(std::function<T> func) {
        std::lock_guard<std::mutex> lock(m_lock);
        this->push_back(func);
        return *this;
    }

    template<typename... Args>
    void callMultithread(Args&&... args) {
        size_t&&                 size_a_cocks = size();
        std::vector<std::thread> cocks;
        cocks.resize(size_a_cocks);
        for (size_t i{}; i < size_a_cocks; ++i) {
            cocks[i] = std::thread((*this)[i], args...);
        }

        for (auto& thr : cocks)
            thr.join();
    }

    template<typename... Args>
    void call(Args&&... args) {
        for (auto& f : *this)
            f(args...);
    }

private:
    std::mutex m_lock;
};
