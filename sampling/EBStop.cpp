#pragma once
#include "EBStop.hpp"

double EBStop::mean() {
    double sum = 0;
    for (int i = 1; i <= t; i++) {
        sum += ebarr[i];
    }
    sum /= t;
    return sum;
}

double EBStop::sd() {
    double ss = 0;
    double mean = EBStop::mean();
    for (int i = 1; i <= t; i++) {
        ss += (mean - ebarr[i]) * (mean - ebarr[i]);
    }
    ss /= t;
    return ss;
}

/*
execute one loop (from t-1 to t, do not change t)
return 0 if ready to stop
*/
int EBStop::loop() {
    double R = 1.0;
    if (t > floor(pow(BETA, k))) {
        k++;
        double alpha = floor(pow(BETA, k)) / floor(pow(BETA, k - 1));
        ebx = -alpha * log((1 / pow(log(BETA, t), 1.1)) / 3)
    }
    double c = sd() * sqrt(2 * ebx / t) + 3 * R * ebx / t;
    lb = max(lb, mean() - c);
    ub = min(ub, mean() + c);
    if ((1 + eps) * lb >= (1 - eps) * ub) {
        return 0;
    }
    return 1;
}

void EBStop::init(double epsilon, double deltad) {
    memset(ebexi, 0, MAX_SAMPLE * sizeof(bool));
    memset(ebarr, 0, MAX_SAMPLE * sizeof(int));
    t = 0;
    k = 0;
    lb = 0;
    ub = INFIN;
    eps = epsilon;
    delta = deltad;
}

/*
add X_rank to EBStop process
return 0 if stop
*/
int EBStop::add(int rank, int x) {
    ebarr[rank] = x;
    ebexi[rank] = true;
    if (rank == t + 1) {
        t = rank;
        if (loop() == 0) {
            return 0;
        }
        while (ebexi[t + 1]) {
            t++;
            if (loop() == 0) {
                return 0;
            }
        }
    }
    return 1;
}

void EBStop::print_res() {
    double est = ((1 + eps) * lb + (1 - eps) * ub) / 2;
    printf("Estimation: %f.", est);
}
