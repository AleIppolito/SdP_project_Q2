/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _GRAPH_H
#define _GRAPH_H
#define THREADS false
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

#include <stdint.h>
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
typedef std::vector<int> EdgeList;		// edge list represented by vertex id list

//CUSTOM STRUCTS
struct Label{
	int pre;
	int post;
	Label(int PRE, int POST) : pre(PRE), post(POST){// @suppress("Class members should be properly initialized")
	};
	Label(){
	};
};
struct Vertex {
	int id;
	int top_level;			// topological level
	int min_int;
	std::vector<Label> labels;
	Vertex(int ID) : id(ID) { // @suppress("Class members should be properly initialized")
		top_level = -1;
	}
	Vertex(){ // @suppress("Class members should be properly initialized")
		top_level = -1;
	};
	int getid(){
		return id;
	}
	Label getLabel(int labelid){
		return labels[labelid];
	}
	int getPost(int labelid){
		return labels[labelid].post;
	}
	int getPre(int labelid){
		return labels[labelid].pre;
	}
	void setLabel(int pre, int post, int id){
		labels[id] = Label(pre,post);
	}
};
struct Node {
	Vertex node;
	EdgeList inList;
	EdgeList outList;
	Node(int ID) : node(ID) { // @suppress("Class members should be properly initialized")
		}
	Node(Vertex v, EdgeList in, EdgeList out) :
<<<<<<< Updated upstream
			node(v) , inList(in), outList(out){
				// @suppress("Class members should be properly initialized")
=======
			vertex(v) , inList(in), outList(out){
					// @suppress("Class members should be properly initialized")
>>>>>>> Stashed changes
		}
	Node(){// @suppress("Class members should be properly initialized")
		};
};


struct VertexCompare {
  bool operator() (const Vertex p1, const Vertex p2) const {
    return p1.id < p2.id;
  }
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
<<<<<<< Updated upstream
		Graph(char*);
		Graph(istream&);
=======
#if THREADS
		Graph(char*,ThreadPool &);
		void readGraph(char*,ThreadPool&);
#else
		Graph(char*);
		void readGraph(char*);
#endif
>>>>>>> Stashed changes
		~Graph();
		bool contains(int src, int trg,int dim);
		bool incrementalContains(int src, int trg, int cur);
<<<<<<< Updated upstream

		void innerRead(std::streampos start, std:: streampos end, char *filename, int n);
		void innerRead2(string buf, int sid, int n);
		void readGraph(char* filename);
		void readGraph2(char* filename);
		//void readGraphQ(char* filename);

=======
>>>>>>> Stashed changes
		void writeGraph(ostream&);
		void printGraph();
		void addEdges(int sid, std::vector<int> tidv);
		void addVertex(int);
		void addEdge(int, int);
		int num_vertices();
		int num_edges();
		GRA& nodes();
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

inline void dothething(Graph& g, int sid, std::vector<int> tid){
	for(auto &i : tid)
		g.addEdge(sid,i);
}

inline void dotheotherthingporcodio(Graph& g, int sid,int tid){
		g.addEdge(sid,tid);
}

inline void worker(Graph &g,std::vector<EdgeList> save){
		for(int i = 0; i <save.size(); i++){
			g.in_edges(i).assign(save[i].begin(), save[i].end());
	
		}
}	
inline void print_graph(Graph& g, string labelfile, int dim){
	ofstream out(labelfile);
	int j = 0;
	while(j < dim){
		out << "Labeling " << j << endl;
		for( int i =0; i < g.num_vertices(); i++){
			out <<  g[i].getPre(j) << " " << g[i].getPost(j) <<  endl;
		}
		j++;
	}
}
#endif
