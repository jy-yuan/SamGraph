#ifndef GRAPH_HPP
#define GRAPH_HPP

#pragma once
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <cassert>

class Graph {
    std::vector<int> vertexes;
    std::vector<int> verExi;
    std::vector<int> verDeg;
    std::vector<int> csrInd;
    std::vector<int> csrList;

   public:
    Graph(){};
    ~Graph(){};
    int init(int m, int mm);
    int init_from_file(const std::string dir = "graph");
    int join(int* zipgraph);
    int* sample(int num);
    bool edge_exist(int u, int v);
    double count();
    double count_triangle();
    double count_three_chain();
    double count_three_motif();
    double count_four_chain();
    double count_five_star();
    int M, MM, N; //M是顶点数，N是边数，MM是采样点数
};

#endif
