#pragma once

#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

using namespace std::literals;

std::string Dir = "Numbers";

std::array<bool, CELL_COUNT * CELL_COUNT> Buffer;

void LoadData(std::string& fPath) {

    std::string data = "";
    std::ifstream file(fPath);

    if (!file.is_open()) {
        std::cout << "Failed to open: " << fPath << '\n';
        return;
    }

    std::getline(file, data);
    file.close();

    constexpr size_t INPUT_SIZE = CELL_COUNT * CELL_COUNT;

    NN.label = data[0] - '0';

    for (size_t i = 0; i < INPUT_SIZE; i++) {
        NN.Input[i].Val = (data[i + 1] == '1') ? 1.f : 0.f;
    }

    //for (unsigned int i = 0; i < T_STEP; i++) {
        NN.Forward();
        NN.Backward();
    //}
}

void LoadFiles(std::string& Path) {
    for (const auto file : std::filesystem::directory_iterator(Path)) {
        if (file.is_regular_file() && file.path().extension() == ".txt") {
            std::string fpath = file.path().string();
            //fpath += file.path().filename().string();
            //std::cout << "Data Loaded : " << fpath << "\n";

            LoadData(fpath);
        }
    }
}

void Load() {

    std::string nfpath = Dir;

    if (std::filesystem::exists(nfpath)) {
        LoadFiles(nfpath);
    }

}

