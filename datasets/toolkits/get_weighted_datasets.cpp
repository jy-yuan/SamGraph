#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RAND_SEED 13
#define MAX_VALUE 79

using namespace std;

typedef uint32_t VertexId;
typedef uint64_t EdgeId;

int main(int argc, char ** argv) {
	if (argc != 5) {
		cout << "./get_weighted_datasets [input-graph(edgelist format)] [output-graph(edgelist format)] [output-graph(biedgelist format)] [output-graph meta-data]" << endl;
	}
	VertexId num_vertices;
	EdgeId num_edges;

	ifstream fin(argv[1]);
	ofstream fout_ascii(argv[2]);
	ofstream fout_binary(argv[3]);

	fin >> num_vertices >> num_edges;
	srand(RAND_SEED);
	VertexId src, dst, value;

	ofstream fout_meta(argv[4]);
	fout_meta << "vertices, edges" << endl;
	fout_meta << num_vertices << ", " << num_edges << endl;
	fout_meta.close();

	fout_ascii << num_vertices << " " << num_edges << endl;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		fin >> src >> dst;
		value = rand() % MAX_VALUE;
		fout_ascii << src << " " << dst << " " << value << endl;
		fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
		fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
		fout_binary.write(reinterpret_cast <const char*> (&value), sizeof(VertexId));
		if ((e_i + 1) % 10000 == 0) {
			cout << "Current processed(binary)(0.1%): " << (e_i + 1) * 1000 / num_edges << endl;
			cout << "\033[F";
		}
	}
	cout << endl;

	fin.close();
	fout_ascii.close();
	fout_binary.close();
	return 0;
}
