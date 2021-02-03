/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _GRAPH_H
#define _GRAPH_H
#define THREADS true
#define DEBUG false
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <sys/time.h>
#include <algorithm>
#include <mutex>
#include <atomic>

#include <condition_variable>
#include "Threadpool.h"
//#include <ext/hash_map>
// #include <sstream>
// #include <set>
// #include <map>
// #include <list>
// #include <deque>
//
// #include <utility>
// #include <cmath>


namespace std {using namespace __gnu_cxx;}
using namespace std;

#define MAX_VAL 10000000
#define MIN_VAL -10000000


struct Vertex {
	int id;
	int top_level;			// topological level
	int min_int;
	std::vector<int> *pre;
	std::vector<int> *post;
	std::vector<int> *middle;

	Vertex(int ID) : id(ID) { // @suppress("Class members should be properly initialized")
		top_level = -1;
	}
	Vertex(){ // @suppress("Class members should be properly initialized")
		top_level = -1;
	};
};

struct VertexCompare {
  bool operator() (const Vertex p1, const Vertex p2) const {
    return p1.id < p2.id;
  }
};

typedef std::vector<int> EdgeList;		// edge list represented by vertex id list
typedef std::vector<Vertex> VertexList;	// vertices list (store real vertex property) indexing by id

struct Node {
	Vertex node;
	EdgeList inList;
	EdgeList outList;
	Node(int ID) : node(ID) { // @suppress("Class members should be properly initialized")
		}
	Node(Vertex v, EdgeList in, EdgeList out) :
			node(v) , inList(in), outList(out){
				// @suppress("Class members should be properly initialized")
		}
	Node(){
		};
};


typedef std::vector<Node> GRA;	// index graph

class Graph {
	private:
		GRA graph;
		int vsize;
		
	public:
		Graph();
		Graph(int);
		Graph(GRA&);
		Graph(char*);
		Graph(istream&);
		~Graph();
		bool contains(int src, int trg,int dim);
		bool incrementalContains(int src, int trg, int cur);

		void innerRead(std::streampos start, std:: streampos end, char *filename, int n);
		void innerRead2(string buf, int sid, int n);
		void readGraph(char* filename);
		void readGraph2(char* filename);
		//void readGraphQ(char* filename);

		void writeGraph(ostream&);
		void printGraph();
		void addVertex(int);
		void addEdge(int, int);
		int num_vertices();
		int num_edges();
		VertexList& vertices();
		EdgeList& out_edges(int);
		EdgeList& in_edges(int);
		int out_degree(int);
		int in_degree(int);
		std::vector<int> getRoots();
		bool hasEdge(int, int);	
		Graph& operator=(const Graph&);
		Vertex& operator[](const int);
		
		void clear();
		// void strTrimRight(string& str);
};	

#endif
