#include <iostream>

#include <chrono>
#include <thread>
#include <vector>
#include <array>

#include <SFML/Graphics.hpp>

constexpr size_t T_STEP = 1;
constexpr size_t CELL_COUNT = 28;
constexpr size_t CELL_SIZE = 20;
constexpr size_t WND_SIZE = CELL_SIZE * CELL_COUNT;

#include "Body.h"
#include "Wire.h"
#include "Cell.h"

Neural<CELL_COUNT * CELL_COUNT, 128, 2, 10> NN;

#include "Learner.h"

struct Block {
    bool on = false;
    sf::RectangleShape shape;
    size_t posx = 0;
    size_t posy = 0;

    void Init(size_t pos_x, size_t pos_y) {

        this->posx = pos_x;
        this->posy = pos_y;

        shape.setSize(sf::Vector2f({CELL_SIZE, CELL_SIZE}));
        shape.setPosition(sf::Vector2f({posx, posy}));
        updateBlock();
    }

    void updateBlock() {
        if (on) shape.setFillColor(sf::Color::Black);
        else shape.setFillColor(sf::Color::White);
    }
    void Draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

int main() {

    Load();

    sf::RenderWindow window(sf::VideoMode({WND_SIZE, WND_SIZE}), "Number Recogonition");

    std::array<Block, CELL_COUNT * CELL_COUNT> Blocks;
    {
        size_t x = 0;
        size_t y = 0;
        for (uint16_t i = 0; i < CELL_COUNT * CELL_COUNT; i++) {
            Block B;
            B.Init(x, y);
            Blocks[i] = B;


            x += CELL_SIZE;
            if (x >= WND_SIZE) {
                x = 0;
                y += CELL_SIZE;
            }
        }

    }
    int8_t num = 0;
    while(window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
                    for (Block& B : Blocks) {
                        B.on = false;
                        B.updateBlock();
                    }
                }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                    num++;
                    if (num > 9) num = 0;
                    std::cout << std::to_string(num) << "\n";
                }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                    num--;
                    if (num < 0) num = 9;
                    std::cout << std::to_string(num) << "\n";
                }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {

                    //pass the request from here to nerual netwrok

                    for (uint16_t i = 0; i < CELL_COUNT * CELL_COUNT; i++) {
                        Block& B = Blocks[i];

                        NN.Input[i].Val = B.on;
                    }

                    //processing
                    NN.Forward();
                    std::cout << NN.GetHypothesis() << "\n";

                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                    continue;
                    std::string path = "Numbers";

                    std::string data = std::to_string(num);
                    for (uint16_t i = 0; i < CELL_COUNT * CELL_COUNT; i++) {
                        Block& B = Blocks[i];
                        data += (B.on) ? '1' : '0';
                    }

                    uint16_t idx = 0;
                    std::filesystem::create_directories(path);
                    if (std::filesystem::exists(path))
                        for (const auto files : std::filesystem::directory_iterator(path))
                            if (files.is_regular_file() && files.path().extension() == ".txt")
                                idx++;

                    path += "/";
                    path += std::to_string(idx);
                    path += ".txt";

                    std::ofstream file(path);
                    if (!file.is_open()) {
                        std::cout << path << "\n";
                        std::cout << "Fail to Open File\n";
                    }
                    file << data;
                    file.close();
                }
            }
        }


        window.clear(sf::Color::Black);

        for (uint16_t i = 0; i < CELL_COUNT * CELL_COUNT; i++) {
            Block& B = Blocks[i];

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                size_t posx = sf::Mouse::getPosition(window).x;
                size_t posy = sf::Mouse::getPosition(window).y;


                if (posx / CELL_SIZE == B.posx / CELL_SIZE &&
                posy / CELL_SIZE == B.posy / CELL_SIZE) {
                    B.on = true;
                    B.updateBlock();
                }
            }

            B.Draw(window);
        }


        window.display();
    }


    return 0;
}
