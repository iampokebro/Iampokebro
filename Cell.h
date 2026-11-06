#pragma once
#include "Wire.h"
#include <vector>
#include <memory>
#include <cstdio>
#include <array>
#include <cassert>
#include <random>

struct CellBase {
    float Bias = static_cast<float>(random(1, 100))/1000.f;
    float Val = static_cast<float>(random(1, 100))/1000.f;
    float error = 0.f;
    virtual ~CellBase() = default;
};

template<uint8_t Max_Size = 4>
struct Cell : public CellBase{

    std::array<std::unique_ptr<Wire>, Max_Size> Wires;
    uint8_t max_size = Max_Size;


public:
    template<uint8_t MS = 4>
    std::unique_ptr<Wire> CreateNetwork(Cell<MS>* Target = nullptr, bool Forward = false) {
        std::unique_ptr<Wire> W(new Wire());

        if (Forward) {
            W->Enter = this;
            W->Exit = Target;
        } else {
            W->Exit = this;
            W->Enter = Target;
        }

        return W;
    }

    template<uint8_t MS>
    void UpdateNetwork(Cell<MS>* Target = nullptr, bool Forward = true, uint16_t& idx = 0) {
        if (idx >= max_size)
        {
            std::cout << "SIZE OUT OF BOUNDS\n";
            return;
        }

        Wires[idx] = CreateNetwork(Target, Forward);
    }
};


