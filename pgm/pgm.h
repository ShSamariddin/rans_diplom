//
// Created by Samariddin Sharipov on 18.03.2022.
//

#ifndef RANS_DIPLOM_PGM_H
#define RANS_DIPLOM_PGM_H
#include <iostream>


enum Result
{
    OK,
    FILE_ERROR,
    OUT_OF_MEMORY,
    WRONG_FORMAT,
    INCORRECT_RESOLUTION,
};

class PGM {
public:
    PGM();
    ~PGM();
    void diff_pgm(PGM& p);
    Result read(const std::string& fileName);
    Result write(const std::string& fileName) const;
    uint8_t get_pixel(int w, int h);
    std::string mode;
    uint32_t width;
    uint32_t height;
    int colorDepth;
    uint8_t* data;
};



#endif //RANS_DIPLOM_PGM_H
