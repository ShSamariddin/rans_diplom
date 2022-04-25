//
// Created by Samariddin Sharipov on 10.04.2022.
//

#include <iostream>
#include <fstream>
#include <time.h>
#include <omp.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <time.h>

#ifndef SYSOUT_F
#define SYSOUT_F(f, ...)      _RPT1( 0, f, __VA_ARGS__ ) // For Visual studio
#endif

#ifndef speedtest__
#define speedtest__(data)   for (long blockTime = NULL; (blockTime == NULL ? (blockTime = clock()) != NULL : false); SYSOUT_F(data "%.9fs", (double) (clock() - blockTime) / CLOCKS_PER_SEC))
#endif

#pragma intrinsic(__rdtsc)
//#include "mpi.h"
#include "pgm/pgm.h"
#include "rANS/rANS.h"
#include "rANS/rans_byte.h"

const int core_num = 12;
using namespace std;
int const step = 6;
int const last_file = 52063;
typedef uint32_t RansState;

char * add_zero(size_t v){
    std::string str = "mgp.";
    while (v != 0){
        str = str + char(48 + v % 10);
        v /= 10;
    }
    while (str.size() != 13){
        str += '0';
    }
    str += "_gmi/dfw/..";
    std::reverse(str.begin(), str.end());
    return strdup(str.c_str());
}
void read(int st_id, int end_id, int thread_id, uint8_t* data){
//    std::cout << thread_id<<' '<<st_id<<' '<<end_id<<'\n';

    PGM pic[3];
    int cnt = 1;
    int cur_pic_num = st_id * 6 + 1;
    pic[0].read(add_zero(cur_pic_num - 6));
    int last_pic_num = end_id * 6 + 1;
    int total_bits = st_id * (400 * 400);
    int kol = 0;
    while(cur_pic_num <= last_pic_num){

        kol++;
        int prev = ((cnt - 1) + 3) % 3;
        pic[cnt].read(add_zero(cur_pic_num));
        pic[prev].diff_pgm(pic[cnt]);
//        for (int i = 0; i < pic[prev].width * pic[prev].height; i ++){
//            data[total_bits] = pic[prev].data[i];
//            total_bits++;
//        }
        cur_pic_num += 6;
        cnt = (cnt  + 1) % 3;
    }
}


int main()
{


    int pgm_num = (52063 - 1) / 6;
    int pgm_per_thread = pgm_num / core_num;
    static const uint32_t prob_bits = 16;
    static const uint32_t prob_scale = 1 << prob_bits;
    int last_pic_num = 52063;
    int cur_pic_num = 7;
    size_t in_size = ((last_pic_num - cur_pic_num) / 6 + 2) * (400 * 400);
    uint8_t* data = new uint8_t[in_size]{0};
    size_t total_bits = 0;
    PGM pic[3];
    std::cout << "here\n";
//    clock_t start = clock();
    time_t n_begin = time(NULL);

#pragma omp parallel num_threads(core_num)
    {
        int thread_id = omp_get_thread_num();
        if(thread_id == core_num - 1){
            read(thread_id * pgm_per_thread + 1,  pgm_num, thread_id,  data);
        } else {
            read(thread_id * pgm_per_thread + 1, (thread_id + 1) * pgm_per_thread, thread_id, data);
        }
        // std::cout << omp_get_thread_num()<<"  Hello thread\n";
    }
    time_t n_end = time(NULL);
    std::cout <<"\ntime reading1: "<< n_end-n_begin<<'\n';

//    std::cout << n_end-n_begin<<' '<<n_end<<' '<<date_time<<'\n';
//    clock_t stop = clock();
//    double elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
//    printf("\ntime_real reading1: %.5f\n", n_end - n_begin);
    int cnt = 1;
    std::cout << "here\n";
    n_begin = time(NULL);
//    start = clock();
    pic[0].read(add_zero(1));
    int kol = 0;
    while(cur_pic_num <= last_pic_num){
        kol++;
        int prev = ((cnt - 1) + 3) % 3;
        pic[cnt].read(add_zero(cur_pic_num));
        pic[prev].diff_pgm(pic[cnt]);
//        for (int i = 0; i < pic[prev].width * pic[prev].height; i ++){
//            data[total_bits] = pic[prev].data[i];
//            total_bits++;
//        }
        cur_pic_num += 6;
        cnt = (cnt  + 1) % 3;
    }
    n_end = time(NULL);
    std::cout <<"\ntime reading1: "<< n_end-n_begin<<'\n';
//    stop = clock();
//    elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
//    printf("\ntime reading2: %.5f\n", elapsed);
    cout << '\n';
    return 0;

    rANS stats;
    std::cout <<total_bits<< " zzzz\n";
    stats.count_freqs(data, size_t (in_size));
    std::cout << "freq\n";
    for (int i = 0; i < MAX_SYM_NUM; i ++){
        std::cout<< stats.freqs[i]<<' ';
    }
    cout << '\n';
//    en.calc_prob_range();
//    std::cout << "freq\n";
//    for (int i = 0; i < MAX_SYM_NUM; i ++){
//        std::cout<< en.prob_range[i]<<' ';
//    }
//    stop = clock();
//    elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
//    printf("\ntime reading: %.5f\n", elapsed);
    std::cout << 'bbbb';
//    start = clock();
    stats.normalize_freqs(prob_scale);
//    stop = clock();
//    printf("\ntime normalizing: %.5f\n", (double) (stop - start) / CLOCKS_PER_SEC);

//    cout << '\n';
//    cout << "norm\n";
//    for(int i = 0; i < MAX_SYM_NUM; i ++){
//        std::cout << stats.prob_range[i]<<' ';
//    }
//    cout <<'\n';

    static size_t out_max_size = in_size; // 32MB
    uint8_t* out_buf = new uint8_t[out_max_size];
    uint8_t* dec_bytes = new uint8_t[in_size];
    std::cout<<"here";
//    start = clock();
    uint8_t cum2sym[prob_scale];
    for (int s=0; s < 256; s++)
        for (uint32_t i=stats.prob_range[s]; i < stats.prob_range[s+1]; i++)
            cum2sym[i] = s;
    std::cout<<"end";
//    stop = clock();
//    printf("\ntime cum2sym: %.5f\n", (double) (stop - start) / CLOCKS_PER_SEC);
    // try rANS encode
    uint8_t *rans_begin;
    RansEncSymbol esyms[256];
    RansDecSymbol dsyms[256];
//    start = clock();

    for (int i=0; i < 256; i++) {
        RansEncSymbolInit(&esyms[i], stats.prob_range[i], stats.freqs[i], prob_bits);
        RansDecSymbolInit(&dsyms[i], stats.prob_range[i], stats.freqs[i]);
    }
//    stop = clock();
//    printf("\ntime preparing: %.5f\n", (double) (stop - start) / CLOCKS_PER_SEC);
    // ---- regular rANS encode/decode. Typical usage.
    std::cout << "eeeee\n";
//    start = clock();

    memset(dec_bytes, 0xcc, in_size);

//    for (int run=0; run < 5; run++) {
//        double start_time = timer();

    RansState rans;
    RansEncInit(&rans);

    uint8_t* ptr = out_buf + out_max_size; // *end* of output buffer
    for (size_t i=in_size; i > 0; i--) { // NB: working in reverse!
        int s = data[i-1];
        RansEncPutSymbol(&rans, &ptr, &esyms[s]);
    }
    RansEncFlush(&rans, &ptr);
    rans_begin = ptr;

//    }
//    stop = clock();
//    printf("\ntime encoded: %.5f\n", (double) (stop - start) / CLOCKS_PER_SEC);
    printf("rANS: %f bytes\n", 100.0 - (((int) (out_buf + out_max_size - rans_begin)) * 100.0)/(in_size * 1.0));
//    start = clock();

    // try rANS decode
//    for (int run=0; run < 5; run++) {
//        double start_time = timer();
//        uint64_t dec_start_time = __rdtsc();

    RansState rans1;
    uint8_t* ptr1 = rans_begin;
    RansDecInit(&rans1, &ptr1);

    for (size_t i=0; i < in_size; i++) {
        uint32_t s = cum2sym[RansDecGet(&rans1, prob_bits)];
        dec_bytes[i] = (uint8_t) s;
        RansDecAdvanceSymbol(&rans1, &ptr1, &dsyms[s], prob_bits);
    }

//        uint64_t dec_clocks = __rdtsc() - dec_start_time;
//        double dec_time = timer() - start_time;
//        printf("%"PRIu64" clocks, %.1f clocks/symbol (%5.1fMiB/s)\n", dec_clocks, 1.0 * dec_clocks / in_size, 1.0 * in_size / (dec_time * 1048576.0));
//    }
//     check decode results
//    stop = clock();

//    printf("\nencoded: %.5f\n", (double) (stop - start) / CLOCKS_PER_SEC);

    if (memcmp(data, dec_bytes, in_size) == 0)
        printf("decode ok!\n");
    else
        printf("ERROR: bad decoder!\n");
}


//73.521903