#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

typedef uint32_t VertexId;
typedef uint64_t EdgeId;

int main(int argc, char ** argv) {
	if (argc != 3) {
		printf("./edgelist_to_arabesque_format [input graph] [output graph]\n");
		exit(-1);
	}
	ifstream fin(argv[1]);
	ofstream fout(argv[2]);

	VertexId num_vertices;
	EdgeId num_edges;
	fin >> num_vertices >> num_edges;

	vector<VertexId> * neighbours = new vector<VertexId>[num_vertices];
	VertexId src, dst;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		if (e_i % 10000 == 0 || e_i == num_edges - 1) {
			printf("\033[F");
			printf("reading graph... %.4f\n", 1. * (e_i + 1) / num_edges);
		}
		fin >> src >> dst;
		neighbours[src].push_back(dst);
		neighbours[dst].push_back(src);
	}
	printf("\n");

	for (VertexId v_i = 0; v_i < num_vertices; ++ v_i) {
		fout << v_i << " " << 1;
		for (VertexId v_j: neighbours[v_i]) {
			fout << " " << v_j;
		}
		fout << endl;
	}

	fin.close();
	fout.close();
	return 0;
}
