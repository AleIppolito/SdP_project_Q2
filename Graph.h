/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _GRAPH_H
#define _GRAPH_H

#include "Threadpool.h"

namespace std {using namespace __gnu_cxx;}
using namespace std;

typedef std::vector<int> EdgeList;		// edge list represented by vertex id list

struct Label{
	int pre;
	int post;
	Label(const int &PRE, const int &POST) : pre(PRE), post(POST){} // @suppress("Class members should be properly initialized")
	Label(){};	// @suppress("Class members should be properly initialized")
};

struct Node {
	std::vector<Label> labels;
	EdgeList inList;
	EdgeList outList;
	Node(EdgeList in, EdgeList out) : inList(in), outList(out) {} // @suppress("Class members should be properly initialized")
	Node(){}; // @suppress("Class members should be properly initialized")
	Label getLabel(const int &labelid) const {return labels[labelid];}
	int getPost(const int &labelid) const {return getLabel(labelid).post;}
	int getPre(const int &labelid) const {return getLabel(labelid).pre;}
	void setLabel(const int &pre, const int &post, const int &id) {labels[id] = Label(pre, post);}
};

typedef std::vector<Node> GRA;	// index graph

class Graph {
	private:
		int dim;
		GRA graph;
	public:
		Graph();
		Graph(GRA&);
		Graph(const string&);
		~Graph();

		void readGraph(const string&);
		void addEdge(const int&, const int&);
		void addVertex(const int&);

		
		bool contains(const int&, const int&, const int&);
		void writeGraph(ostream&);
		int num_vertices() const;
		std::vector<int> getRoots() const;
		bool hasEdge(int&, int&);
		int num_edges() const;
		//GRA& vertexes();
		EdgeList& out_edges(const int&);
		EdgeList& in_edges(const int&);
		int out_degree(const int&);
		int in_degree(const int&);
		void printGraph();
		Graph& operator=(const Graph&);
		Node& operator[](const int &vid);
		void clear();
};

/*
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
 */

#endif
