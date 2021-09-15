#ifndef EBSTOP_HPP
#define EBSTOP_HPP
#pragma once
#include <cmath>
#include <cstdio>
#include <cstring>

#define MAX_SAMPLE 1000
#define BETA 10
#define INFIN 1 << 20

/*
Empirical Bernstein Stopping
@inproceedings{mnih2008empirical,
  title={Empirical bernstein stopping},
  author={Mnih, Volodymyr and Szepesv{\'a}ri, Csaba and Audibert, Jean-Yves},
  booktitle={Proceedings of the 25th international conference on Machine
learning}, pages={672--679}, year={2008}
}
*/

class EBStop {
   public:
    static EBStop& get_instance() {
        static EBStop ebins;
        return ebins;
    }
    void init(double epsilon, double deltad);
    int add(int rank, int x);
    void print_res();

   private:
    bool ebexi[MAX_SAMPLE];
    int ebarr[MAX_SAMPLE];
    int t = 0;
    int k = 0;
    double lb = 0;
    double ub = INFIN;
    double eps = 0.05;
    double delta = 0.05;
    double ebx = 1;
    EBStop();
    ~EBStop(){};
    double mean();
    double sd();
    int loop();
};

#endif
