#ifndef IESTOP_HPP
#define IESTOP_HPP
#pragma once
#include <cmath>
#include <cstdio>
#include <cstring>

#define MAX_SAMPLE 10000
#define MIN_SAMPLE 10
#define INFIN 1 << 20

/*
Interval Estimation Stopping
based on central limit theorem
*/

class IEStop {
   public:
    static IEStop& get_instance();
    void init(double alphaa = 0.05, double deltad = 0.05);
    int add(int rank, double x);
    void print_res();

   private:
    bool ieexi[MAX_SAMPLE];
    double iearr[MAX_SAMPLE];
    int t = 0;
    int k = 0;
    double lb = 0;
    double ub = INFIN;
    double alpha;
    double delta;
    double iex = 1;
    IEStop(){};
    ~IEStop(){};
    double mean();
    double sd();
    double zscore(double a);
    int loop();
};

#endif
