#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>


/* This Single File Was Just a Piece of Vibe Code From Entire Project,
The project was completed so i deiced to just to copy it and past it,'
100 percent vibe no coder involvement notice.
*/
template<uint16_t IL, uint16_t HL, uint16_t HRL, uint16_t OL>
bool Store(const Neural<IL, HL, HRL, OL>& nn, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }

    // Write architecture header metadata
    file << IL << "," << HL << "," << HRL << "," << OL << "\n";

    // 1. Store Input Layer
    for (uint16_t i = 0; i < IL; ++i) {
        file << nn.Input[i].Val << "," << nn.Input[i].Bias;
        for (uint16_t h = 0; h < HL; ++h) {
            if (nn.Input[i].Wires[h]) {
                file << "," << nn.Input[i].Wires[h]->Weight;
            } else {
                file << ",0.0";
            }
        }
        file << "\n";
    }

    // 2. Store Hidden Layers
    for (uint16_t hr = 0; hr < HRL; ++hr) {
        for (uint16_t h = 0; h < HL; ++h) {
            const auto& cell = nn.Hidden[hr][h];
            file << cell.Val << "," << cell.Bias;
            uint16_t target_len = (hr < HRL - 1) ? HL : OL;
            for (uint16_t k = 0; k < target_len; ++k) {
                if (cell.Wires[k]) {
                    file << "," << cell.Wires[k]->Weight;
                } else {
                    file << ",0.0";
                }
            }
            file << "\n";
        }
    }

    // 3. Store Output Layer
    for (uint16_t o = 0; o < OL; ++o) {
        file << nn.Output[o].Val << "," << nn.Output[o].Bias << "\n";
    }

    file.close();
    return true;
}

template<uint16_t IL, uint16_t HL, uint16_t HRL, uint16_t OL>
bool Sloader(Neural<IL, HL, HRL, OL>& nn, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << "\n";
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) return false;

    // Read and verify architecture dimensions
    std::stringstream header_ss(line);
    std::string token;
    uint16_t f_il, f_hl, f_hrl, f_ol;

    std::getline(header_ss, token, ','); f_il = static_cast<uint16_t>(std::stoi(token));
    std::getline(header_ss, token, ','); f_hl = static_cast<uint16_t>(std::stoi(token));
    std::getline(header_ss, token, ','); f_hrl = static_cast<uint16_t>(std::stoi(token));
    std::getline(header_ss, token, ','); f_ol = static_cast<uint16_t>(std::stoi(token));

    if (f_il != IL || f_hl != HL || f_hrl != HRL || f_ol != OL) {
        std::cerr << "Dimension mismatch between file header and Neural template parameters!\n";
        return false;
    }

    auto parse_float = [](std::stringstream& ss, float& val) {
        std::string s;
        if (std::getline(ss, s, ',')) {
            val = std::stof(s);
        }
    };

    // 1. Load Input Layer
    for (uint16_t i = 0; i < IL; ++i) {
        if (!std::getline(file, line)) return false;
        std::stringstream ss(line);
        parse_float(ss, nn.Input[i].Val);
        parse_float(ss, nn.Input[i].Bias);
        for (uint16_t h = 0; h < HL; ++h) {
            if (nn.Input[i].Wires[h]) {
                parse_float(ss, nn.Input[i].Wires[h]->Weight);
            } else {
                float dummy;
                parse_float(ss, dummy);
            }
        }
    }

    // 2. Load Hidden Layers
    for (uint16_t hr = 0; hr < HRL; ++hr) {
        for (uint16_t h = 0; h < HL; ++h) {
            if (!std::getline(file, line)) return false;
            std::stringstream ss(line);
            auto& cell = nn.Hidden[hr][h];
            parse_float(ss, cell.Val);
            parse_float(ss, cell.Bias);
            uint16_t target_len = (hr < HRL - 1) ? HL : OL;
            for (uint16_t k = 0; k < target_len; ++k) {
                if (cell.Wires[k]) {
                    parse_float(ss, cell.Wires[k]->Weight);
                } else {
                    float dummy;
                    parse_float(ss, dummy);
                }
            }
        }
    }

    // 3. Load Output Layer
    for (uint16_t o = 0; o < OL; ++o) {
        if (!std::getline(file, line)) return false;
        std::stringstream ss(line);
        parse_float(ss, nn.Output[o].Val);
        parse_float(ss, nn.Output[o].Bias);
    }

    file.close();
    return true;
}
