#include "Graph.hpp"

inline int getint(FILE *stream) {
    char c = fgetc(stream);
    int symbol = 1;
    while ((c < '0' || c > '9') && c != '-') {
        c = fgetc(stream);
    }
    if (c == '-') {
        symbol = -1;
        c = fgetc(stream);
    }
    int x = c - '0';
    c = fgetc(stream);
    while ('0' <= c && c <= '9') {
        x = x * 10 + c - '0';
        c = fgetc(stream);
    }
    return x * symbol;
}

/*
just init and allocate space
*/
int Graph::init(int m, int mm) {
    M = m;
    MM = mm;
    N = 0;
    verExi.resize(M);
    verDeg.resize(M);
    csrInd.resize(M + 1);
    return 0;
}

/*
read a partial graph from doc like:
M N
u1 v1
u2 v2
...
u_n v_n

where M is # of vertexes and N is # of edges
*/
int Graph::init_from_file(const std::string dir) {
    FILE *pFile = fopen(dir.c_str(), "r");
    M = getint(pFile);
    N = getint(pFile);
    printf("M:%d,N:%d\n", M, N);
    // fscanf(pFile, "%d %d", &M, &N);
    verExi.resize(M);
    verDeg.resize(M);
    csrInd.resize(M + 1);
    // csrList.reserve(N);
    csrList.resize(N);
    int u, v;
    int tmp = 0;
    for (int i = 0; i < N; i++) {
        // fscanf(pFile, "%d %d", &u, &v);
        u = getint(pFile);
        v = getint(pFile);
        if (u > tmp) {
            for (int j = tmp + 1; j <= u; j++) {
                csrInd[j + 1] = csrInd[j];
            }
            tmp = u;
        }
        verExi[u] = 1;
        verDeg[u]++;
        // csrList.insert(csrList.begin() + csrInd[u + 1]++, v);
        csrList[csrInd[u + 1]++] = v;
    }
    for (int i = tmp + 1; i <= M; i++) {
        csrInd[i + 1] = csrInd[i];
    }
    for (int i = 0; i < M; i++) {
        if (verExi[i]) {
            vertexes.push_back(i);
        }
    }
    assert(csrInd[M] == N);
    return 0;
}

/*
join another partial graph into current graph
zipgraph is a data structure of graph (easy to trans by MPI)
contains,
int: num of vertex
int: num of edge
array: whether the vertex exits TODO:optimize
array: csr index
array: csr list
*/
int Graph::join(int *zipgraph) {
    int m = zipgraph[0];
    if (m != M) {
        printf("ERROR: Vertex of two graph are not the same!\n");
        return -1;
    }
    int n = zipgraph[1];
    int *newExi = zipgraph + 2;
    int *newInd = zipgraph + 2 + m;
    int *newList = zipgraph + 2 + m + m + 1;
    for (int i = 0; i < m; i++) {
        verExi[i] = (int)(newExi[i] || verExi[i]);
    }
    for (int i = 0; i < m; i++) {
        // printf("csrInd %d = %d\n", i, csrInd[i]);
        csrInd[i + 1] = csrInd[i] + verDeg[i];
        if (newExi[i]) {  // useless, for speed up
            for (int j = newInd[i]; j < newInd[i + 1]; j++) {
                csrList.insert(csrList.begin() + csrInd[i + 1]++, newList[j]);
            }
            verDeg[i] = verDeg[i] + newInd[i + 1] - newInd[i];
        }
    }
    assert(csrList.size() == N + n);
    assert(csrInd[m] == csrList.size());
    N += n;
    return 0;
}

/*
sample [num] vertexes from graph
in the structure of zipgraph
*/
int *Graph::sample(int num) {
    int *newExi = (int *)calloc(M, sizeof(int));
    int *newInd = (int *)calloc(M + 1, sizeof(int));
    std::vector<int> newList;
    srand(rand());
    int a;
    for (int i = 0; i < num; i++) {
        a = rand() % vertexes.size();
        while (newExi[vertexes[a]]) {
            a = rand() % vertexes.size();
        }
        newExi[vertexes[a]] = 1;
    }
    for (int i = 0; i < M; i++) {
        newInd[i + 1] = newInd[i];
        if (newExi[i]) {
            for (int j = csrInd[i]; j < csrInd[i + 1]; j++) {
                newList.insert(newList.begin() + newInd[i + 1]++, csrList[j]);
            }
        }
    }
    int *sam = (int *)malloc((2 + M + M + 1 + newList.size()) * sizeof(int));
    sam[0] = M;
    sam[1] = newList.size();
    memcpy(sam + 2, newExi, sizeof(int) * M);
    memcpy(sam + 2 + M, newInd, sizeof(int) * (M + 1));
    memcpy(&sam[2 + M + M + 1], newList.data(), sizeof(int) * newList.size());
    return sam;
}

bool Graph::edge_exist(int u, int v) {
    for (int i = csrInd[u]; i < csrInd[u + 1]; i++) {
        if (csrList[i] == v) {
            return true;
        }
    }
    return false;
}

// naive implementation, count edges
// for users to implement
double Graph::count() {
    double count = 0;
    // printf("verExi:");
    for (int i = 0; i < M; i++) {
        if (verExi[i]) {
            for (int j = csrInd[i]; j < csrInd[i + 1]; j++) {
                if (verExi[csrList[j]]) count++;
            }
        }
    }
    // printf("\n");
    double samplerate = (double)MM / M;
    return count / (samplerate * samplerate);
}

// naive implementation of triangle counting
double Graph::count_triangle() {
    double count = 0;
    int a, b, c;
    for (int i = 0; i < M; i++) {
        a = i;
        if (verExi[i] && verDeg[i] >= 2) {
            for (int j = csrInd[i]; j < csrInd[i + 1]; j++) {
                b = csrList[j];
                if (b < a || !verExi[b]) {
                    continue;
                }
                for (int k = j + 1; k < csrInd[i + 1]; k++) {
                    c = csrList[k];
                    if (c < b || !verExi[c]) {
                        continue;
                    }
                    if (edge_exist(b, c)) {
                        count++;
                    }
                }
            }
        }
    }
    double samplerate = (double)MM / M;
    return count / (samplerate * samplerate * samplerate);
}

double Graph::count_three_chain() {
    double count = 0;
    int a, b, c;
    for (int i = 0; i < M; i++) {
        a = i;
        if (verExi[i]) {
            for (int j = csrInd[i]; j < csrInd[i + 1]; j++) {
                b = csrList[j];
                if (b < a || !verExi[b]) {
                    continue;
                }
                for (int k = j + 1; k < csrInd[i + 1]; k++) {
                    c = csrList[k];
                    if (c <= a || !verExi[c] || edge_exist(c, a)) {
                        continue;
                    }
                    count++;
                }
            }
        }
    }
    double samplerate = (double)MM / M;
    return count / (samplerate * samplerate * samplerate);
}

double Graph::count_three_motif() {
    return count_triangle() + count_three_chain();
}

double Graph::count_four_chain() {
    double count = 0;
    int a, b, c, d;
    for (int i = 0; i < M; i++) {
        a = i;
        if (!verExi[a]) {
            continue;
        }
        for (int j = csrInd[a]; j < csrInd[a + 1]; j++) {
            b = csrList[j];
            if (!verExi[b]) {
                continue;
            }
            for (int k = csrInd[b]; k < csrInd[b + 1]; k++) {
                c = csrList[k];
                if (!verExi[c] || c == a) {
                    continue;
                }
                for (int l = csrInd[c]; l < csrInd[c + 1]; l++) {
                    d = csrList[l];
                    if (verExi[d] && d > a && d != b) {
                        count++;
                    }
                }
            }
        }
    }
    double samplerate = (double)MM / M;
    return count / (samplerate * samplerate * samplerate * samplerate);
}

double Graph::count_five_star() {
    double count = 0;
    for (int i = 0; i < M; i++) {
        if (!verExi[i] || verDeg[i] < 5) {
            continue;
        }
        int deg = 0;
        for (int j = csrInd[i]; j < csrInd[i + 1]; j++) {
            if (verExi[csrList[j]]) {
                deg++;
            }
        }
        if (deg >= 5) {
            count += deg * (deg - 1) * (deg - 2) * (deg - 3) * (deg - 4) / 120;
        }
    }
    double samplerate = (double)MM / M;
    return count /
           (samplerate * samplerate * samplerate * samplerate * samplerate);
}