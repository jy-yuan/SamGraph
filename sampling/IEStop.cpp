#pragma once
#include "IEStop.hpp"

IEStop& IEStop::get_instance() {
    static IEStop ieins;
    return ieins;
}

double IEStop::mean() {
    double sum = 0;
    for (int i = 1; i <= t; i++) {
        sum += iearr[i];
    }
    sum /= t;
    return sum;
}

double IEStop::sd() {
    double ss = 0;
    double mean = IEStop::mean();
    for (int i = 1; i <= t; i++) {
        ss += (mean - iearr[i]) * (mean - iearr[i]);
    }
    return sqrt(ss/t);
}

double IEStop::zscore(double a) {
    if (a == 0.1) {
        return 1.282;
    } else if (a == 0.05) {
        return 1.645;
    } else if (a == 0.025) {
        return 1.96;
    } else if (a == 0.01) {
        return 2.326;
    } else if (a == 0.005) {
        return 2.576;
    } else {
        return 1;
    }
}

/*
execute one loop (from t-1 to t, do not change t)
return 0 if ready to stop
*/
int IEStop::loop() {
    printf("t = %d, sd = %f, mean = %f.\n", t, sd(), mean());
    if (t < MIN_SAMPLE) {
        return 1;
    }
    if (zscore(alpha / 2) * sd() / (sqrt(t) * mean()) > delta) {
        return 1;
    }
    return 0;
}

void IEStop::init(double alphaa, double deltad) {
    memset(ieexi, 0, MAX_SAMPLE * sizeof(bool));
    memset(iearr, 0, MAX_SAMPLE * sizeof(double));
    t = 0;
    k = 0;
    lb = 0;
    ub = INFIN;
    alpha = alphaa;
    delta = deltad;
}

/*
add X_rank to IEStop process
return 0 if stop
*/
int IEStop::add(int rankk, double x) {
    int rank = t + 1; //for fun
    iearr[rank] = x;
    ieexi[rank] = true;
    if (rank > MAX_SAMPLE) {
        printf("[debug] t:%d\n", t);
        return 0;
    }
    if (rank == t + 1) {
        t = rank;
        if (loop() == 0) {
            return 0;
        }
        while (ieexi[t + 1]) {
            t++;
            if (loop() == 0) {
                return 0;
            }
        }
    }
    return 1;
}

void IEStop::print_res() {
    double est = mean();
    printf("***************Estimation result: %f***************\n", est);
}
