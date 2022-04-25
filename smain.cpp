#include <iostream>
#include <fstream>
#include "pgm/pgm.h"
#include "rANS/rANS.h"
#include "rANS/rans_byte.h"

using namespace std;
int const step = 6;
int const last_file = 52063;
typedef uint32_t RansState;

char * add_zero(int v){
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


// ---- rANS encoding/decoding with alias table

static inline void RansEncPutAlias(RansState* r, uint8_t** pptr, rANS* const syms, int s, uint32_t scale_bits)
{
    // renormalize
    uint32_t freq = syms->freqs[s];
    RansState x = RansEncRenorm(*r, pptr, freq, scale_bits);

    // x = C(s,x)
    // NOTE: alias_remap here could be replaced with e.g. a binary search.
    *r = ((x / freq) << scale_bits) + syms->alias_remap[(x % freq) + syms->prob_range[s]];
}

static inline uint32_t RansDecGetAlias(RansState* r, rANS* const syms, uint32_t scale_bits)
{
    RansState x = *r;

    // figure out symbol via alias table
    uint32_t mask = (1u << scale_bits) - 1; // constant for fixed scale_bits!
    uint32_t xm = x & mask;
    uint32_t bucket_id = xm >> (scale_bits - rANS::LOG2NSYMS);
    uint32_t bucket2 = bucket_id * 2;
    if (xm < syms->divider[bucket_id])
        bucket2++;

    // s, x = D(x)
    *r = syms->slot_freqs[bucket2] * (x >> scale_bits) + xm - syms->slot_adjust[bucket2];
    return syms->sym_id[bucket2];
}

int main()
{
//    PGM pict[10000];
//    PGM pgm;
//    std::ofstream out;

    static const uint32_t prob_bits = 14;
    static const uint32_t prob_scale = 1 << prob_bits;
    int x = 403;
    int cnt = 0;
    size_t in_size = 800;
//    size_t in_size = 1377760000;
    uint8_t* data = new uint8_t[in_size]{1};
//    while(x <= 52063){
//        PGM p1;
//        PGM p2;
//        p1.read(add_zero(x));
//        if (x > 1) {
//            p2.read(add_zero(x - 6));
//            p1.diff_pgm(p2);
//        }
//        for (int i = 0; i < p1.width * p1.height; i ++){
//            data[total_bits] = p1.data[i];
//            total_bits++;
//        }
//        x += 6;
//    }
    cout << '\n';
    rANS stats;
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
    stats.normalize_freqs(prob_scale);
    cout << '\n';
    cout << "norm\n";
    for(int i = 0; i < MAX_SYM_NUM; i ++){
        std::cout << stats.prob_range[i]<<' ';
    }
    cout <<'\n';
    stats.make_alias_table();
    static size_t out_max_size = 1377760000; // 32MB
    uint8_t* out_buf = new uint8_t[out_max_size];
    uint8_t* dec_bytes = new uint8_t[in_size];



    memset(dec_bytes, 0xcc, in_size);
    uint8_t *rans_begin;

    // try interleaved rANS encode
//    printf("\ninterleaved rANS encode:\n");
    for (int run=0; run < 5; run++) {
//        double start_time = timer();
        uint64_t enc_start_time = __rdtsc();

        RansState rans0, rans1;
        RansEncInit(&rans0);
        RansEncInit(&rans1);

        uint8_t* ptr = out_buf + out_max_size; // *end* of output buffer

        // odd number of bytes?
        if (in_size & 1) {
            int s = data[in_size - 1];
            RansEncPutAlias(&rans0, &ptr, &stats, s, prob_bits);
        }

        for (size_t i=(in_size & ~1); i > 0; i -= 2) { // NB: working in reverse!
            int s1 = data[i-1];
            int s0 = data[i-2];
            RansEncPutAlias(&rans1, &ptr, &stats, s1, prob_bits);
            RansEncPutAlias(&rans0, &ptr, &stats, s0, prob_bits);
        }
        RansEncFlush(&rans1, &ptr);
        RansEncFlush(&rans0, &ptr);
        rans_begin = ptr;

        uint64_t enc_clocks = __rdtsc() - enc_start_time;
//        double enc_time = timer() - start_time;
//        printf("%"PRIu64" clocks, %.1f clocks/symbol (%5.1fMiB/s)\n", enc_clocks, 1.0 * enc_clocks / in_size, 1.0 * in_size / (enc_time * 1048576.0));
    }

    std::cout << in_size<<'\n';
    unsigned int real_size = 0;
    for (size_t i = 0; i < 256; i ++){
        real_size += stats.freqs[i];
    }
    std::cout << "real size: "<<real_size<<'\n';
//    fp.write((char*)rans_begin, (int) (out_buf + out_max_size - rans_begin));
    printf("rANS: %f bytes\n", 100.0 - (((int) (out_buf + out_max_size - rans_begin)) * 100.0)/(in_size * 1.0));

    // try interleaved rANS decode
    for (int run=0; run < 5; run++) {
//        double start_time = timer();
        uint64_t dec_start_time = __rdtsc();

        RansState rans0, rans1;
        uint8_t* ptr = rans_begin;
        RansDecInit(&rans0, &ptr);
        RansDecInit(&rans1, &ptr);

        for (size_t i=0; i < (in_size & ~1); i += 2) {
            uint32_t s0 = RansDecGetAlias(&rans0, &stats, prob_bits);
            uint32_t s1 = RansDecGetAlias(&rans1, &stats, prob_bits);
            dec_bytes[i+0] = (uint8_t) s0;
            dec_bytes[i+1] = (uint8_t) s1;
            RansDecRenorm(&rans0, &ptr);
            RansDecRenorm(&rans1, &ptr);
        }

        // last byte, if number of bytes was odd
        if (in_size & 1) {
            uint32_t s0 = RansDecGetAlias(&rans0, &stats, prob_bits);
            dec_bytes[in_size - 1] = (uint8_t) s0;
            RansDecRenorm(&rans0, &ptr);
        }

        uint64_t dec_clocks = __rdtsc() - dec_start_time;
//        double dec_time = timer() - start_time;
//        printf("%"PRIu64" clocks, %.1f clocks/symbol (%5.1fMB/s)\n", dec_clocks, 1.0 * dec_clocks / in_size, 1.0 * in_size / (dec_time * 1048576.0));
    }

    // check decode results
    if (memcmp(data, dec_bytes, in_size) == 0)
        printf("decode ok!\n");
    else
        printf("ERROR: bad decoder!\n");

    delete[] out_buf;
    delete[] dec_bytes;
    delete[] data;
    return 0;
}


