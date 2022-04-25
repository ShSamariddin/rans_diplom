//
// Created by Samariddin Sharipov on 26.03.2022.
//

#include <cassert>
#include "rANS.h"

void rANS::count_freqs(uint8_t const *in, size_t nbytes) {
    for (int i = 0; i < MAX_SYM_NUM; i++)
        freqs[i] = 0;

    for (size_t i = 0; i < nbytes; i++)
        freqs[in[i]]++;
}

void rANS::calc_prob_range() {
    prob_range[0] = 0;
    for (int i = 0; i < MAX_SYM_NUM; i++)
        prob_range[i + 1] = prob_range[i] + freqs[i];
}

void rANS::normalize_freqs(uint32_t target_total) {
    assert(target_total >= MAX_SYM_NUM);
    calc_prob_range();
    uint32_t symbols_cnt = prob_range[MAX_SYM_NUM];

    // увеличиваем значение в prob_range но при этом постараемся сохранить  соотношение между элементами
    for (int i = 1; i <= MAX_SYM_NUM; i++)
        prob_range[i] = ((uint64_t) target_total * prob_range[i]) / symbols_cnt;

    // Если значение какого-то символа не равно нулю, но при это его range равно нулю,
    // то попытаемся отнять у кого-то единицу и сделать наш  ненулевым
    for (int i = 0; i < MAX_SYM_NUM; i++) {
        if (freqs[i] && prob_range[i + 1] == prob_range[i]) {
            // символ i подходить под наши условия, значит мы должны найти символ с которого отнимем единицу

            uint32_t best_freq = ~0u;
            int best_steal = -1;
            for (int j = 0; j < MAX_SYM_NUM; j++) {
                uint32_t freq = prob_range[j + 1] - prob_range[j];
                if (freq > 1 && freq < best_freq) {
                    best_freq = freq;
                    best_steal = j;
                }
            }
            // если не нашли символа у которого freq строго больше 1
            assert(best_steal != -1);

            // Здесь мы попытаемся отнять единицу и присвоит себе
            if (best_steal < i) {
                for (int j = best_steal + 1; j <= i; j++)
                    prob_range[j]--;
            } else {
                assert(best_steal > i);
                for (int j = i + 1; j <= best_steal; j++)
                    prob_range[j]++;
            }
        }
    }

    // проверяем инвариант
    assert(prob_range[0] == 0 && prob_range[MAX_SYM_NUM] == target_total);

    //обновляем частоту
    for (int i = 0; i < MAX_SYM_NUM; i++) {
        freqs[i] = prob_range[i + 1] - prob_range[i];
    }
}

