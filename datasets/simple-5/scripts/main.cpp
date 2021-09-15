#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <fstream>
#include <utility>
#include <set>

using namespace std;

typedef uint32_t VertexId;
typedef uint64_t EdgeId;

int main(int argc, char ** argv) {
	ofstream fout_ascii("../dataset/simple-5.edgelist");
	ofstream fout_binary("../dataset/simple-5.biedgelist");

	srand(time(NULL));
	set<pair<VertexId, VertexId>> used_edges;
	const VertexId num_vertices = 5;
	const EdgeId num_edges = 5;
	fout_ascii << num_vertices << " " << num_edges << endl;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		while (true) {
			VertexId src = rand() % num_vertices;
			VertexId dst = rand() % num_vertices;
			if (src == dst) continue; // self-loops are not allowed
			if (used_edges.find(make_pair(src, dst)) != used_edges.end()) {
				continue; // duplicated edges are not allowed
			}
			used_edges.insert(make_pair(src, dst));
			used_edges.insert(make_pair(dst, src));
			fout_ascii << src << " " << dst << endl;
			fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
			fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
			break;
		}
	}

	fout_ascii.close();
	fout_binary.close();
	return 0;
}
