#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

typedef unsigned int VertexId;
typedef unsigned long long EdgeId;

struct EdgeUnit {
	VertexId src;
	VertexId dst;
};

VertexId num_vertices;
EdgeId num_edges;

EdgeUnit * edges;

int main(int argc, char ** argv) {
	if (argc != 3) {
		cout << "usage: ./edgelist_to_biedgelist [input graph] [output graph]\n";
		exit(-1);
	}
	char * input_graph = argv[1];
	char * output_graph = argv[2];
	ifstream fin_ascii(input_graph);
	ofstream fout_binary(output_graph);
	fin_ascii >> num_vertices >> num_edges;
	edges = new EdgeUnit[num_edges];
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		fin_ascii >> edges[e_i].src >> edges[e_i].dst;
	}
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		fout_binary.write(reinterpret_cast <const char*> (&edges[e_i].src), sizeof(VertexId));
		fout_binary.write(reinterpret_cast <const char*> (&edges[e_i].dst), sizeof(VertexId));
	}
	delete [] edges;
	fin_ascii.close();
	fout_binary.close();
	return 0;
}
