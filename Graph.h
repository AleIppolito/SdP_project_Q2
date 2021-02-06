/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _GRAPH_H
#define _GRAPH_H
#define THREADS true
#define DEBUG false

#include "Threadpool.h"
// #include <future>

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

struct Node {
	std::vector<Label> labels;
	EdgeList inList;
	EdgeList outList;
	Node(EdgeList in, EdgeList out) :
			inList(in), outList(out){
					// @suppress("Class members should be properly initialized")
		}
	Node(){// @suppress("Class members should be properly initialized")
		};
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



typedef std::vector<Node> GRA;	// index graph


class Graph {
	private:
		GRA graph;
	public:
		Graph();
		Graph(GRA&);
#if THREADS
		Graph(char*,ThreadPool &);
		void readGraph(char*,ThreadPool&);
#else
		Graph(char*);
		void readGraph(char*);
#endif
		~Graph();

		bool contains(int src, int trg,int dim);
		bool incrementalContains(int src, int trg, int cur);
		void writeGraph(ostream&);
		void printGraph();
		void addEdges(int sid, std::vector<int> tidv);
		void addVertex(int);
		void addEdge(int, int);
		int num_vertices();
		int num_edges();
		GRA& vertexes();
		EdgeList& out_edges(int);
		EdgeList& in_edges(int);
		int out_degree(int);
		int in_degree(int);
		std::vector<int> getRoots();
		bool hasEdge(int, int);	
		Graph& operator=(const Graph&);
		Node& operator[](const int);
		void clear();
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
