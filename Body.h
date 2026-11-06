#include "Wire.h"
#include "Cell.h"

#include <cstring>

#include <array>
#include <algorithm>
#include <cmath>
inline void Softmax(std::vector<float>& logits);

float ReLU();

constexpr float learing_rate = 0.001f;

template<uint16_t IL = 16, uint16_t HL = 16, uint16_t HRL = 16, uint16_t OL = 16>
struct Neural {
    uint16_t Input_len;
    uint16_t Hidden_len;
    uint16_t Horizontal_len;
    uint16_t Output_len;

    static constexpr uint16_t MWC = (HL > OL) ? HL : OL;

    Neural() : Input_len(IL), Hidden_len(HL), Output_len(OL), Horizontal_len(HRL) {
        InitilizeCells();
    }

    //Cell<0> Input[IL];
    std::array<Cell<HL>, IL> Input;
    std::array<std::array<Cell<HL>, HL>, HRL> Hidden;
    std::array<Cell<1>, OL> Output;


    void connectInput() {
        for (uint16_t i = 0; i < IL; i++) {
            for (uint16_t h = 0; h < HL; h++) {
                Input[i].UpdateNetwork(&Hidden[0][h], true, h);
            }
        }
    }
    void connectHidden() {
        for (uint16_t hr = 0; hr < HRL - 1; hr++) {
            for (uint16_t h = 0; h < HL; h++) {
                Cell<HL> *Hid = &Hidden[hr][h];
                for (uint16_t hk = 0; hk < HL; hk++) {
                    Hid->UpdateNetwork(&Hidden[hr+1][hk], true, hk);
                }
            }
        }
    }

    void connectOutput() {
        for (uint16_t i = 0; i < HL; i++) {
            Cell<HL> *Hid = &Hidden[HRL-1][i];
            for (uint16_t o = 0; o < OL; o++) {
                Hid->UpdateNetwork(&Output[o], true, o);
            }
        }
    }

    void InitilizeCells() {

        connectInput();
        connectHidden();
        connectOutput();
    }




    //MODEL- HYPOTHESIS
    void Forward() {
        //input
        for (size_t h = 0; h < HL; h++) {
            Cell<HL> *Hid = &Hidden[0][h];
            Hid->Val = 0.f;
            for (size_t i = 0; i < IL; i++) {
                std::unique_ptr<Wire>& W = Input[i].Wires[h];
                Hid->Val += Input[i].Val * W->Weight;
            }
            Hid->Val += Hid->Bias;
            Hid->Val = std::max(0.f, Hid->Val);
        }

        //hidden
        for (int k = 1; k < HRL; k++) {
            for (size_t h = 0; h < HL; h++) {
                Cell<HL> *Hid = &Hidden[k][h];
                Hid->Val = 0.f;
                for (size_t i = 0; i < HL; i++) {
                    Cell<HL>& H = Hidden[k-1][i];
                    std::unique_ptr<Wire>& W = H.Wires[h];
                    Hid->Val += H.Val * W->Weight;
                }
                Hid->Val += Hid->Bias;
                Hid->Val = std::max(0.f, Hid->Val);
            }
        }

        //Output
        std::vector<float> logits(OL);
        for (size_t h = 0; h < OL; h++) {
            Cell<1> *OP = &Output[h];
            OP->Val = 0.f;
            for (size_t i = 0; i < HL; i++) {
                Cell<HL>& H = Hidden[HRL - 1][i];
                std::unique_ptr<Wire>& W = H.Wires[h];
                OP->Val += H.Val * W->Weight;
            }
            OP->Val += OP->Bias;
            logits[h] = OP->Val;
        }

        Softmax(logits);
        for (size_t h = 0; h < OL; h++) {
            Output[h].Val = logits[h];
        }
    }

    int GetHypothesis() {
        int best_class = 0;
        float max_prob = Output[0].Val;
        for (int i = 1; i < OL; i++) {
            if (Output[i].Val > max_prob) {
                max_prob = Output[i].Val;
                best_class = i;
            }
        }
        return best_class;
    }

    size_t label = 0;
    //BACKPROPOGATION
    void Backward() {
        for (uint16_t i = 0; i < OL; i++) Output[i].error = 0.f;
        for (uint16_t l = 0; l < HRL; l++)
            for (uint16_t i = 0; i < HL; i++) Hidden[l][i].error = 0.f;
        for (uint16_t i = 0; i < IL; i++) Input[i].error = 0.f;

        //output
        for (uint16_t i = 0; i < OL; i++) {
            Cell<1>* OP = &Output[i];
            float target = (i == label) ? 1.0f : 0.0f;

            OP->error = (OP->Val - target);

            for (uint16_t h = 0; h < HL; h++) {
                Cell<HL>& H = Hidden[HRL-1][h];
                std::unique_ptr<Wire>& W = H.Wires[i];
                float relu = (H.Val > 0.f) ? 1.f : 0.f;
                float oldWeight = W->Weight;

                H.error += OP->error * oldWeight * relu;

                W->Weight -= learing_rate * OP->error * H.Val;
            }
            OP->Bias -= learing_rate * OP->error;
        }

        //hidden
        for (int l = HRL - 1; l >= 1; l--) {
            for (uint16_t i = 0; i < HL; i++) {
                Cell<HL>* OP = &Hidden[l][i];

                for (uint16_t h = 0; h < HL; h++) {
                    Cell<HL>& H = Hidden[l - 1][h];
                    std::unique_ptr<Wire>& W = H.Wires[i];
                    float relu = (H.Val > 0.f) ? 1.f : 0.f;

                    float oldWeight = W->Weight;

                    H.error += OP->error * oldWeight * relu;

                    W->Weight -= learing_rate * OP->error * H.Val;
                }
                OP->Bias -= learing_rate * OP->error;
                OP->error = 0.f;
            }
        }

        //INPUT
        for (uint16_t i = 0; i < HL; i++) {
            Cell<HL>& OP = Hidden[0][i];

            for (uint16_t k = 0; k < IL; k++) {
                Cell<HL>& H = Input[k];
                std::unique_ptr<Wire>& W = H.Wires[i];
                float relu = (H.Val > 0.f) ? 1.f : 0.f;
                float oldWeight = W->Weight;

                H.error += OP.error * oldWeight * relu;

                W->Weight -= learing_rate * OP.error * H.Val;
            }
            OP.Bias -= learing_rate * OP.error;
        }
    }

};

inline void Softmax(std::vector<float>& logits) {
    float max_val = logits[0];
    for (size_t i = 1; i < logits.size(); i++) {
        if (logits[i] > max_val) max_val = logits[i];
    }

    float sum = 0.0f;
    for (size_t i = 0; i < logits.size(); i++) {
        logits[i] = std::exp(logits[i] - max_val);
        sum += logits[i];
    }

    for (size_t i = 0; i < logits.size(); i++) {
        logits[i] /= sum;
    }
}
