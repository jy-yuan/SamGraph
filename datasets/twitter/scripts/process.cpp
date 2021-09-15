#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include <fstream>

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
} __attribute__((packed));

EdgeUnit * edges;

const VertexId num_vertices = 41652230;
EdgeId num_edges = (EdgeId) 1468364884;
const EdgeId CHUNKSIZE = num_edges / 2 + 1;

int main(int argc, char ** argv) {
	edges = new EdgeUnit[CHUNKSIZE];

	VertexId src, dst;
	FILE * fp;
	EdgeId num_read_edges = 0;
	EdgeId num_not_self_loop_edges = 0;

	fp = fopen("../dataset/twitter-2010.txt", "r");
	cout << "checking num_edges && num_vertices" << endl;
	while (fscanf(fp, "%u%u", &src, &dst) != EOF) { // there is self-loops! 
		//if (num_read_edges >= num_edges) {
		//	cout << src << " " << dst << endl;
		//}
		assert(src >= 0 && src < num_vertices);
		assert(dst >= 0 && dst < num_vertices);
		if (src != dst) ++ num_not_self_loop_edges;
		num_read_edges ++;
		if (num_read_edges % 10000 == 0 || num_read_edges == num_edges) {
			cout << "Current read(0.1%): " << num_read_edges * 1000 / num_edges << endl;
			cout << "\033[F";
		}
	}
	//assert(num_not_self_loop_edges == num_edges);
	cout << num_not_self_loop_edges << " " << num_edges << " " << num_read_edges << endl;
	num_edges = num_read_edges;
	fclose(fp);
	cout << endl;
	cout << "finished checking num_edges && num_vertices" << endl;

	EdgeId num_edges_first_halt = CHUNKSIZE;
	EdgeId num_edges_second_halt = num_edges - CHUNKSIZE; 
	
	//EdgeId num_edges_first_halt = 1 << 20;
	//EdgeId num_edges_second_halt = 1 << 20;

	// checking if there is 
	cout << "reading edges in first halt..." << endl;
	fp = fopen("../dataset/twitter-2010.txt", "r");
	for (EdgeId e_i = 0; e_i < num_edges_first_halt; ++ e_i) {
		fscanf(fp, "%u%u", &src, &dst);
		edges[e_i].src = src;
		edges[e_i].dst = dst;
		if (e_i % 1000 == 0 || e_i + 1 == num_edges_first_halt) {
			cout << "Current read(0.1%): " << e_i * 1000 / num_edges_first_halt<< endl;
			cout << "\033[F";
		}
	}
	cout << endl;

	cout << "sorting edges in first halt..." << endl;
	sort(edges, edges + num_edges_first_halt);

	cout << "asserting that there is no duplicated edge[first-half, first-halt]..." << endl;
	for (EdgeId e_i = 1; e_i < num_edges_first_halt; ++ e_i) {
		assert(! (edges[e_i] == edges[e_i - 1]));
	}

	cout << "asserting that there is no duplicated edge[first-half, second-half].." << endl;
	EdgeUnit tmp;
	for (EdgeId e_i = 0; e_i < num_edges_second_halt; ++ e_i) {
		fscanf(fp, "%u%u", &tmp.src, &tmp.dst);
		{
			if (tmp == edges[0]) assert(false);
			if (tmp == edges[num_edges_first_halt - 1]) assert(false);
			if (edges[0] < tmp && tmp < edges[num_edges_first_halt - 1]) {
				EdgeId left = 0;
				EdgeId right = num_edges_first_halt - 1;
				EdgeId mid;
				while (right - left > 1) {
					mid = (left + right) >> 1;
					if (tmp == edges[mid]) assert(false); // duplicated-edges
					if (edges[mid] < tmp) left = mid; else right = mid;
				}
			}
		}
		if (e_i % 1000 == 0 || e_i + 1 == num_edges_second_halt) {
			cout << "Current processed(0.1%): " << e_i * 1000 / num_edges_second_halt << endl;
			cout << "\033[F";
		}
	}
	cout << endl;

	fclose(fp);

	fp = fopen("../dataset/twitter-2010.txt", "r");

	cout << "skipping the edges in first half..." << endl;
	for (EdgeId e_i = 0; e_i < num_edges_first_halt; ++ e_i) {
		fscanf(fp, "%u%u", &tmp.src, &tmp.dst);
		if (e_i % 1000 == 0 || e_i + 1 == num_edges_first_halt) {
			cout << "Current skipped(0.1%): " << e_i * 1000 / num_edges_first_halt << endl;
			cout << "\033[F";
		}
	}
	cout << endl;

	cout << "reading the edges in second half..." << endl;
	for (EdgeId e_i = 0; e_i < num_edges_second_halt; ++ e_i) {
		fscanf(fp, "%u%u", &edges[e_i].src, &edges[e_i].dst);
		if (e_i % 1000 == 0 || e_i + 1 == num_edges_second_halt) {
			cout << "Current skipped(0.1%): " << e_i * 1000 / num_edges_second_halt << endl;
			cout << "\033[F";
		}
	}
	cout << endl;

	cout << "sorting edges in second halt..." << endl;
	sort(edges, edges + num_edges_second_halt);

	cout << "asserting that there is no duplicated edge[second-half, second-halt]..." << endl;
	for (EdgeId e_i = 1; e_i < num_edges_second_halt; ++ e_i) {
		assert(! (edges[e_i] == edges[e_i - 1]));
	}

	fclose(fp);

	ofstream fout_ascii("../dataset/twitter-2010.edgelist");
	ofstream fout_binary("../dataset/twitter-2010.biedgelist");
	ofstream fout_meta("../dataset/twitter-2010.csv");

	fout_meta << "vertices, edges" << endl;
	fout_meta << num_vertices << ", " << num_not_self_loop_edges << endl; 
	fout_ascii << num_vertices << " " << num_not_self_loop_edges << endl;

	fout_meta.close();

	fp = fopen("../dataset/twitter-2010.txt", "r");
	num_read_edges = 0;
	cout << "generating datasets(edgelist && biedgelist format)..." << endl;
	for (EdgeId e_i = 0; e_i < num_edges; ++ e_i) {
		fscanf(fp, "%u%u", &src, &dst);
		if (src != dst) {
			++ num_read_edges;
			fout_ascii << src << " " << dst << endl;
			fout_binary.write(reinterpret_cast <const char*> (&src), sizeof(VertexId));
			fout_binary.write(reinterpret_cast <const char*> (&dst), sizeof(VertexId));
		}
		if (e_i % 1000 == 0 || e_i + 1 == num_edges) {
			cout << "Current written(0.1%): " << e_i * 1000 / num_edges << endl;
			cout << "\033[F";
		}
	}
	assert(num_read_edges == num_not_self_loop_edges);
	fclose(fp);

	fout_ascii.close();
	fout_binary.close();

	delete [] edges;
	return 0;
}
