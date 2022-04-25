//
// Created by Samariddin Sharipov on 26.03.2022.
//

#ifndef RANS_DIPLOM_RANS_H
#define RANS_DIPLOM_RANS_H
#include <iostream>
#include <vector>

static const int MAX_SYM_NUM = 1 << 8;

class rANS {
public:
    static const int LOG2NSYMS = 8;

    rANS()=default;
    ~rANS()=default;
    void count_freqs(uint8_t const* in, size_t nbytes);
    void calc_prob_range();
    void normalize_freqs(uint32_t target_total);
    std::vector<uint32_t> ans;

    uint32_t freqs[MAX_SYM_NUM];
    uint32_t prob_range[MAX_SYM_NUM + 1];

};


#endif //RANS_DIPLOM_RANS_H
