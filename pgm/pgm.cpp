//
// Created by Samariddin Sharipov on 18.03.2022.
//

#include "pgm.h"
#include <fstream>

PGM::~PGM() {
    delete data;
}

PGM::PGM() {
    width = -1;
    height = -1;
    mode = "None";
    colorDepth = 0;
    data = nullptr;
}

Result PGM::read(const std::string &fileName) {
    delete data;
    std::ifstream input;
    input.open(fileName, std::ios_base::in | std::ios_base::binary);
    if (!input.is_open()) {
        return FILE_ERROR;
    }
    input >> mode >> width >> height >> colorDepth;
    if (mode != "P5") {
        return WRONG_FORMAT;
    }
    data = new uint8_t[height * width + 1];
    if (data == nullptr) {
        return OUT_OF_MEMORY;
    }
    char trash[1];
    input.read((char *) trash, 1);
    input.read((char *) data, height * width);
    input.close();
    return OK;
}

Result PGM::write(const std::string &fileName) const {
    std::ofstream output;
    output.open(fileName, std::ios_base::out | std::ios_base::binary);
    if (!output.is_open()) {
        return FILE_ERROR;
    }
    output << "P5\n" << width << ' ' << height << '\n' << colorDepth<<'\n';
    output.write((char *) data, width * height);
    output.close();
    return OK;
}

uint8_t PGM::get_pixel(int w, int h) {
    return data[h * width + w];
}

void PGM::diff_pgm(PGM& p){
    for (int h = 0; h < height; h ++){
        for (int w = 0; w < width; w ++) {
            data[h * width + w] = data[h * width + w] - p.get_pixel(w, h);
//            if (data[h * width + w] < 10){
//                data[h * width + w] = 0;
//            }
        }
    }
}
//1388169850
//782445179
//73.742921
//78.695865
//21.929455