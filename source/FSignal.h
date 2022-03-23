#pragma once
template<typename T>
class FSignal : public std::vector<std::function<T>> {
public:
    inline FSignal& operator+=(std::function<T> func) {
        std::lock_guard<std::mutex> lock(m_lock);
        this->push_back(func);
        return *this;
    }

private:
    std::mutex m_lock;
};
