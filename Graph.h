/*
 * @file Graph.h
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @copyright Copyright (c) 2021
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include "Threadpool.h"

typedef std::vector<int> EdgeList;		// edge list represented by vertex id list

struct Label{
	int pre;
	int post;

	Label(){};
	Label(const int &PRE, const int &POST) : pre(PRE), post(POST){} // @suppress("Class members should be properly initialized")
	
};

struct Node {
	std::vector<Label> labels;
	EdgeList inList;
	EdgeList outList;

	Node(){}; 
	Node(EdgeList in, EdgeList out) : inList(in), outList(out) {} // @suppress("Class members should be properly initialized")
	

	Label getLabel(const int &labelid) const {return labels[labelid];}
	int getPost(const int &labelid) const {return getLabel(labelid).post;}
	int getPre(const int &labelid) const {return getLabel(labelid).pre;}
	void setLabel(const int &pre, const int &post, const int &id) {labels[id] = Label(pre, post);}
};
typedef std::vector<Node> GRA;	

class Graph {
	private:
		int dim;
		GRA graph;
	public:
		Graph();
		Graph(GRA&);
		Graph(const std::string&, ThreadPool&);
		~Graph();

		void makeinList(GRA &);

		EdgeList getRoots() const;
		EdgeList& out_edges(const int&);
		EdgeList& in_edges(const int&);

		int out_degree(const int&);
		int in_degree(const int&);
		int num_edges() const;
		int num_vertices() const;
		
		void printGraph();
		void writeGraph(std::ostream&);

		Graph& operator=(const Graph&);
		Node& operator[](const int &vid);

		void clear();
};

void readChunk(const std::string&, GRA &, const int, const int);

#endif /* GRAPH_H_ */
