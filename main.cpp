#include <iostream>
#include <fstream>
#include <time.h>
#include <omp.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>

//#include "mpi.h"
#include "pgm/pgm.h"
#include "rANS/rANS.h"
#include "rANS/rans_byte.h"

using namespace std;


typedef uint32_t RansState;
rANS stats;
int freqs[12][300];
char *add_zero(size_t v) {
    std::string str = "mgp.";
    while (v != 0) {
        str = str + char(48 + v % 10);
        v /= 10;
    }
    while (str.size() != 9) {
        str += '0';
    }
    str += "_tluser/tuo/..";
    std::reverse(str.begin(), str.end());
    return strdup(str.c_str());
}

void read(int id,  int st_id, int end_id, uint8_t *data) {
    PGM * pic = new PGM[3];
    int cnt = 1;
    int cur_pic_num = st_id * step + 1;
    pic[0].read(add_zero(cur_pic_num - step));
    int last_pic_num = end_id * step + 1;
    size_t total_bits = st_id * (pic[0].height * pic[0].width);
    int kol = 0;
    while (cur_pic_num < last_pic_num) {

        kol++;
        int prev = ((cnt - 1) + 3) % 3;
        pic[cnt].read(add_zero(cur_pic_num));
        pic[prev].diff_pgm(pic[cnt]);
        for (int i = 0; i < pic[prev].width * pic[prev].height; i++) {
            data[total_bits] = pic[prev].data[i];
            total_bits++;
        }
        cur_pic_num += step;
        cnt = (cnt + 1) % 3;
    }
}

uint8_t *encode(size_t r, size_t l, const uint8_t *data, RansEncSymbol esyms[], uint8_t *ptr) {
    RansState rans;
    RansEncInit(&rans);
    for (size_t i = r; i > l; i--) { // NB: working in reverse!
        int s = data[i - 1];
        RansEncPutSymbol(&rans, &ptr, &esyms[s]);
    }
    RansEncFlush(&rans, &ptr);
    return ptr;
}


int main(int argc, char *argv[]) {

    std::ofstream out;
    out.open("somefile.bin", std::ios::out | std::ios::binary);
    time_t s_begin = time(NULL);
    int pgm_num = (last_file - first_file) / step + 1;
    int pgm_per_thread = pgm_num / core_num;
    PGM pic;
    pic.read(add_zero(first_file));
    pgm_width = pic.width;
    pgm_height = pic.height;
    std::cout << pic.height<<' '<<pic.width<<'\n';
    out.write((char *) &pgm_num, 4);
    out.write((char *) &pic.height, 4);
    out.write((char *) &pic.width, 4);
    static const uint32_t prob_bits = 16;
    static const uint32_t prob_scale = 1 << prob_bits;
    size_t in_size = (pgm_num + 10) * (pic.height * pic.width);
    size_t lack_bits = core_num - ((in_size - 1) % core_num + 1);
    in_size += lack_bits;//increase size till  divided by core_num
    uint8_t *data = new uint8_t[in_size]{0};
    for (int i = 0; i < MAX_SYM_NUM; i++)
        stats.freqs[i] = 0;
    for (int i = 0; i < pic.width * pic.height; i++) {
        data[i] = pic.data[i];
        stats.freqs[pic.data[i]]++;
    }
#pragma omp parallel num_threads(core_num)
    {
        int thread_id = omp_get_thread_num();
        if (thread_id == core_num - 1) {
            read(thread_id, thread_id * pgm_per_thread + 1, pgm_num, data);
        } else {
            read(thread_id, thread_id * pgm_per_thread + 1, (thread_id + 1) * pgm_per_thread, data);
        }
    }
    time_t s_end = time(NULL);
    std::cout << "\ntime reading : " << s_end - s_begin << '\n';

    stats.count_freqs(data, size_t(in_size));
    s_begin = time(NULL);
    stats.normalize_freqs(prob_scale);
    out.write((char *) &stats.freqs, 4 * 256);
    out.write((char *) stats.prob_range, 4 * 257);
//    s_begin = time(NULL);

    RansEncSymbol esyms[256];
    for (int i = 0; i < 256; i++) {
        RansEncSymbolInit(&esyms[i], stats.prob_range[i], stats.freqs[i], prob_bits);
    }

    uint8_t *out_buf[core_num];
    uint8_t *ptr[core_num];
    size_t bits_per_core = in_size / core_num;
    size_t out_max_size_per_core = bits_per_core;

    for (int core_id = 0; core_id < core_num; core_id++) {
        out_buf[core_id] = new uint8_t[bits_per_core];
    }

#pragma omp parallel num_threads(core_num)
    {
        int core_id = omp_get_thread_num();
        ptr[core_id] = out_buf[core_id] + out_max_size_per_core; // *end* of output buffer
        size_t l = core_id * bits_per_core;
        size_t r = (core_id + 1) * bits_per_core;
        ptr[core_id] = encode(r, l, data, esyms, ptr[core_id]);
    }
    int total_res = 0;
    for (int core_id = 0; core_id < core_num; core_id++) {
        uint32_t si = out_buf[core_id] + bits_per_core - ptr[core_id];
        total_res += si;
        out.write((char *) &si, 4);
        out.write((char *) ptr[core_id], out_buf[core_id] + bits_per_core - ptr[core_id]);
    }
    out.close();
    std::cout << total_res << '\n';
    s_end = time(NULL);
    printf("rans size : %d\n", total_res);
    printf("rans size : %ud\n", in_size);
    printf("rANS: %f bytes\n", (total_res * 100.0) / (in_size * 1.0));
    std::cout << "\ntime encode : " << s_end - s_begin << '\n';
}
