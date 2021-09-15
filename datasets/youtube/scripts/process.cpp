#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

#include <stdint.h>
#include <assert.h>

using namespace std;

typedef uint32_t VertexId;
typedef uint64_t EdgeId;

struct Edge {
	VertexId src;
	VertexId dst;
	Edge(VertexId src_, VertexId dst_): src(src_), dst(dst_) {
	}
};

VertexId vertices;
EdgeId edges;

vector<Edge> saved_edges;
vector<VertexId> saved_vertices;
set<VertexId> saved_vertices_set;

int main(int argc, char ** argv) {
	ifstream fin("../dataset/com-youtube.ungraph.txt");
	ofstream fout_ascii("../dataset/youtube.edgelist");
	ofstream fout_binary("../dataset/youtube.biedgelist");

	string tmp;
	getline(fin, tmp); // skip the 1st line
	getline(fin, tmp); // skip the 2nd line

	for (int i = 0; i < 5; ++ i) {
		fin >> tmp;
		if (i == 2) {
			vertices = atoi(tmp.c_str());
		} 
		if (i == 4) {
			edges = atoi(tmp.c_str());
		}
	}

	cout << "Number of vertices: " << vertices << endl;
	cout << "Number of edges: " << edges << endl;

	ofstream fout_meta("../dataset/youtube.csv");
	fout_meta << "vertices, edges" << endl;
	fout_meta << vertices << ", " << edges << endl;
	fout_meta.close();

	getline(fin, tmp);
	getline(fin, tmp);

	saved_vertices_set.clear();
	saved_vertices.clear();
	VertexId largest_vertex_id = 0;

	EdgeId read_edges = 0;
	VertexId last_src = 0, last_dst = 0;
	VertexId src, dst;
	while (!(fin >> src >> dst).eof()) {
		read_edges ++;
		if (saved_vertices_set.find(src) == saved_vertices_set.end()) {
			saved_vertices_set.insert(src);
			saved_vertices.push_back(src);
			largest_vertex_id = max(largest_vertex_id, src);
		}
		if (saved_vertices_set.find(dst) == saved_vertices_set.end()) {
			saved_vertices_set.insert(dst);
			saved_vertices.push_back(dst);
			largest_vertex_id = max(largest_vertex_id, dst);
		}
		//assert(src >= 0 && src < vertices);
		//assert(dst >= 0 && dst < vertices);
		if (read_edges % 10000 == 0) {
			cout << "Current read(0.1%): " << read_edges * 1000 / edges << endl;
			cout << "\033[F";
		}
		if (src == last_src && dst == last_dst) continue; // delete duplicated edges
		last_src = src;
		last_dst = dst;
		if (src >= dst) continue; // delete self-loop && duplicated edges
		saved_edges.push_back(Edge(src, dst));
		//cout << src << " " << dst << endl;
	}
	assert(edges == read_edges);

	assert(saved_vertices.size() == vertices);
	cout << "unique vertices: " << saved_vertices.size() << endl;
	VertexId * mapping = new VertexId[largest_vertex_id + 1];
	sort(saved_vertices.begin(), saved_vertices.end());
	for (VertexId i = 0; i < saved_vertices.size(); ++ i) {
		mapping[saved_vertices[i]] = i;
	}
	for (EdgeId e_i = 0, e_size = saved_edges.size(); e_i < e_size; ++ e_i) {
		saved_edges[e_i].src = mapping[saved_edges[e_i].src];
		saved_edges[e_i].dst = mapping[saved_edges[e_i].dst];
	}

	cout << "Number of edges saved: " << saved_edges.size() << endl;

	fout_ascii << vertices << " " << saved_edges.size() << endl;
	for (EdgeId i = 0, j = saved_edges.size(); i < j; ++ i) {
		fout_ascii << saved_edges[i].src << " " << saved_edges[i].dst << endl;
	}

	for (EdgeId i = 0, j = saved_edges.size(); i < j; ++ i) {
		VertexId src = saved_edges[i].src;
		VertexId dst = saved_edges[i].dst;
		fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
		fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
	}

	delete [] mapping;

	fin.close();
	fout_ascii.close();
	fout_binary.close();
	return 0;
}
