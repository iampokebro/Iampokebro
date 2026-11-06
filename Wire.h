#pragma once
#include <random>

inline int random(int x, int y) {
    std::random_device rd;
    thread_local static std::mt19937 engine(rd());
    std::uniform_int_distribution<int> dist(x, y);
    return dist(engine);
}


struct CellBase;

struct Wire {
    float Weight = static_cast<float>(random(1, 100))/1000.f;

    CellBase* Enter = nullptr;
    CellBase* Exit = nullptr;
};
