//
// Created by Samariddin Sharipov on 12.04.2022.
//

#include <iostream>
#include <fstream>
#include "pgm/pgm.h"
#include "rANS/rANS.h"
#include "rANS/rans_byte.h"
#include <omp.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>

char *add_zero(size_t v) {
    std::string str = "mgp.";
    while (v != 0) {
        str = str + char(48 + v % 10);
        v /= 10;
    }
    //add zero before number
//    while (str.size() != 13) {
//        str += '0';
//    }
    str += "_gmi/ser/..";
    std::reverse(str.begin(), str.end());
    return strdup(str.c_str());
}

void write(int st_id, int end_id, uint32_t width, uint32_t height, uint8_t *data) {
    PGM pic;
    int cnt = 1;
    int cur_pic_num = st_id * step + 1;
    int last_pic_num = end_id * step + 1;
    size_t total_bits = st_id * (pgm_height * pgm_height);
    pic.width = width;
    pic.height = height;
    pic.colorDepth = 255;
    pic.mode = "P5";
    pic.data = new uint8_t[height * width + 1];
    for (int i = 0; i < pic.width * pic.height; i++) {
        pic.data[i] = data[total_bits];
        total_bits++;
    }
    pic.write(add_zero(cur_pic_num));
    cur_pic_num += step;
    int kol = 0;
    //1388480004
    while (cur_pic_num < last_pic_num) {
        for (int i = 0; i < pic.width * pic.height; i++) {
            pic.data[i] -= data[total_bits];
            total_bits++;
        }
        pic.write(add_zero(cur_pic_num));
        cur_pic_num += step;
    }
}

int main(int argc, char *argv[]) {

    std::ifstream file("somefile.bin", std::ios::in | std::ios::binary | std::ios::ate);
    static const uint32_t prob_bits = 16;
    static const uint32_t prob_scale = 1 << prob_bits;

    if (file.is_open()) {
        size_t size = file.tellg();
        uint32_t pgm_num;
        uint32_t width;
        uint32_t height;
        uint32_t q;
        uint8_t *ptr[core_num];
        rANS stats;
        time_t s_begin = time(NULL);
        file.seekg(0, std::ios::beg);
        file.read((char *) &pgm_num, 4);
        file.read((char *) &height, 4);
        file.read((char *) &width, 4);
        file.read((char *) stats.freqs, 4 * 256);
        file.read((char *) stats.prob_range, 4 * 257);//prob_rage of 0 is 0
        for (int i = 0; i < core_num; i++) {
            uint32_t si;
            file.read((char *) &si, 4);
            ptr[i] = new uint8_t[si];
            file.read((char *) ptr[i], si);
        }
        file.close();
        time_t s_end = time(NULL);

        std::cout << "\ntime read : " << s_end - s_begin << '\n';

        s_begin = time(NULL);
        size_t in_size = (pgm_num + 1) * (pgm_width * pgm_height);
        size_t lack_bits = core_num - ((in_size - 1) % core_num + 1);
        in_size += lack_bits;//increase size till  divided by core_num
        uint8_t *dec_bytes = new uint8_t[in_size];
        std::cout << in_size<<'\n';
        uint8_t cum2sym[prob_scale];
        for (int s = 0; s < 256; s++)
            for (uint32_t i = stats.prob_range[s]; i < stats.prob_range[s + 1]; i++)
                cum2sym[i] = s;

        RansDecSymbol dsyms[256];
        for (int i = 0; i < 256; i++) {
            RansDecSymbolInit(&dsyms[i], stats.prob_range[i], stats.freqs[i]);
        }
        RansDecSymbol copy_dsyms[core_num][256];
        for(int core_id = 0; core_id < core_num; core_id++){
            for (int i = 0; i < 256; i++) {
                copy_dsyms[core_id][i] = dsyms[i];
            }
        }
#pragma omp parallel num_threads(core_num)
        {

            int core_id = omp_get_thread_num();
            size_t bits_per_core = in_size / core_num;
            size_t l = core_id * bits_per_core;
            size_t r = (core_id + 1) * bits_per_core;
            const uint32_t copy_prob_bits = prob_bits;
            RansState rans;
            RansDecInit(&rans, &ptr[core_id]);

            for (size_t i = l; i < r; i++) {
                uint32_t s = cum2sym[RansDecGet(&rans, copy_prob_bits)];
                dec_bytes[i] = (uint8_t) s;
                RansDecAdvanceSymbol(&rans, &ptr[core_id], &copy_dsyms[core_id][s], copy_prob_bits);
            }
        }
        int pgm_per_thread = pgm_num / core_num;
        s_end = time(NULL);
        std::cout << "\ntime decode : " << s_end - s_begin << '\n';
        s_begin = time(NULL);
        write(0, pgm_num, width, height, dec_bytes);



        s_end = time(NULL);
        std::cout << "\ntime write : " << s_end - s_begin << '\n';
        file.close();
    }

}
