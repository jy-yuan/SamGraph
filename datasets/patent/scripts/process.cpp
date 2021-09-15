#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include <fstream>
#include <string.h>

using namespace std;

typedef uint32_t VertexId;
typedef uint64_t EdgeId;

struct EdgeUnit {
	VertexId src;
	VertexId dst;
	bool operator < (const EdgeUnit &e) const {
		if (src < e.src) return true;
		if (src > e.src) return false;
		if (dst < e.dst) return true;
		return false;
	}
	bool operator == (const EdgeUnit &e) const {
		return (src == e.src && dst == e.dst);
	}
	bool operator != (const EdgeUnit &e) const {
		return !(*this == e);
	}
} __attribute__((packed));

EdgeUnit * edge_list;
const VertexId num_vertices = 3774768;
const EdgeId num_edges = 16518948;

bool * is_used;
VertexId * mapping;
const VertexId max_vertex_id = 10000000;

int main(int argc, char ** argv) {
	ifstream fin("../dataset/cit-Patents.txt");

	string tmp;
	getline(fin, tmp); // skip 
	getline(fin, tmp); // skip 
	getline(fin, tmp); // skip 
	getline(fin, tmp); // skip 

	EdgeId read_num_edges = 0;
	edge_list = new EdgeUnit[num_edges];
	VertexId src, dst;
	is_used = new bool [max_vertex_id];
	memset(is_used, false, sizeof(bool) * max_vertex_id);
	mapping = new VertexId [max_vertex_id];
	while (!(fin >> src >> dst).eof()) {
		assert(read_num_edges < num_edges);
		if (src > dst) swap(src, dst);
		//cout << src << " " << num_vertices << endl;
		//cout << dst << " " << num_vertices << endl;

		assert(0 <= src && src < max_vertex_id);
		assert(0 <= dst && dst < max_vertex_id);
		is_used[src] = true;
		is_used[dst] = true;

		edge_list[read_num_edges].src = src;
		edge_list[read_num_edges].dst = dst;
		++ read_num_edges;

		//cout << "Reading dataset: " << 1. * read_num_edges / num_edges << endl;
		//cout << "\033[F";
	}
	cout << endl;

	VertexId new_vertex_id = 0;
	for (VertexId v_i = 0; v_i < max_vertex_id; ++ v_i) {
		if (is_used[v_i]) {
			mapping[v_i] = new_vertex_id ++;
		}
	}
	assert(new_vertex_id <= num_vertices);
	assert(new_vertex_id == num_vertices);

	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		edge_list[e_i].src = mapping[edge_list[e_i].src];
		edge_list[e_i].dst = mapping[edge_list[e_i].dst];
	}

	assert(num_edges == read_num_edges);
	sort(edge_list, edge_list + num_edges);

	ofstream fout_ascii("../dataset/patents.edgelist");
	ofstream fout_binary("../dataset/patents.biedgelist");
	EdgeId num_valid_edges = 0;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		if (e_i%10000==0) {
		cout << "Writing dataset(binary): " << 1. * (e_i + 1) / num_edges << endl;
		cout << "\033[F";}
		if (e_i == 0 || (edge_list[e_i] != edge_list[e_i - 1])) {
			src = edge_list[e_i].src;
			dst = edge_list[e_i].dst;
			if (src != dst) {
				fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
				fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
				++ num_valid_edges;
			}
		}
	}
	cout << endl;
	cout << "num_edges = " << num_edges << " " << "num_valid_edges = " << num_valid_edges << endl;

	fout_ascii << num_vertices << " " << num_valid_edges << endl;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		if (e_i%10000==0) {
		cout << "Writing dataset(ascii): " << 1. * (e_i + 1) / num_edges << endl;
		cout << "\033[F";}
		if (e_i == 0 || (edge_list[e_i] != edge_list[e_i - 1])) {
			src = edge_list[e_i].src;
			dst = edge_list[e_i].dst;
			if (src != dst) {
				fout_ascii << src << " " << dst << endl;
			}
		}
	}
	cout << endl;
	fout_ascii.close();
	fout_binary.close();

	ofstream fout_meta("../dataset/patents.csv");
	fout_meta << "vertices, edges" << endl; 
	fout_meta << num_vertices << ", " << num_valid_edges << endl;
	fout_meta.close();

	fin.close();
	return 0;
}
