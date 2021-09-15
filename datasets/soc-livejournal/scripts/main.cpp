#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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

int main(int argc, char ** argv) {
	ifstream fin("../dataset/soc-LiveJournal1.txt");
	ofstream fout_ascii("../dataset/live-journal.edgelist");
	ofstream fout_binary("../dataset/live-journal.biedgelist");
	ofstream fout_meta("../dataset/live-journal.csv");

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

	fout_meta << "vertices, edges" << endl;
	fout_meta << vertices << ", " << edges << endl;
	fout_meta.close();

	getline(fin, tmp);
	getline(fin, tmp);

	EdgeId read_edges = 0;
	VertexId last_src = 0, last_dst = 0;
	VertexId src, dst;
	while (!(fin >> src >> dst).eof()) {
		read_edges ++;
		assert(src >= 0 && src < vertices);
		assert(dst >= 0 && dst < vertices);
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

	cout << "Number of edges saved: " << saved_edges.size() << endl;

	fout_ascii << vertices << " " << saved_edges.size() << endl;
	for (EdgeId i = 0, j = saved_edges.size(); i < j; ++ i) {
		if (i % 10000 == 0) {
			cout << "Current saved(ascii)(0.1%): " << i * 1000 / j << endl;
			cout << "\033[F";
		}
		fout_ascii << saved_edges[i].src << " " << saved_edges[i].dst << endl;
	}

	for (EdgeId i = 0, j = saved_edges.size(); i < j; ++ i) {
		if (i % 10000 == 0) {
			cout << "Current saved(binary)(0.1%): " << i * 1000 / j << endl;
			cout << "\033[F";
		}
		VertexId src = saved_edges[i].src;
		VertexId dst = saved_edges[i].dst;
		fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
		fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
	}

	fin.close();
	fout_ascii.close();
	fout_binary.close();
	return 0;
}
