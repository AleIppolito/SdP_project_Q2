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
/**
 * @brief Used as alias for a vector of edges 
 */
typedef std::vector<int> EdgeList;		// edge list represented by vertex id list
/**
 * @brief Label struct for keeping pre and post order labels 
 */
struct Label{
	int pre;
	int post;

	Label(){}; // @suppress("Class members should be properly initialized")
	Label(const int &PRE, const int &POST) : pre(PRE), post(POST){}
};

/**
 * @brief Node struct
 * keeps vertex data such as labels outEdges and inEdges
 */
struct Node {
	std::vector<Label> labels;
	EdgeList outList;
#if BIDI
	EdgeList inList;
#else
	bool isRoot;
#endif

#if BIDI
	Node(const EdgeList in, const EdgeList out) : inList(in), outList(out) {} // @suppress("Class members should be properly initialized")
	Node(){}
#else
	Node(const EdgeList out) : isRoot(true), outList(out) {} // @suppress("Class members should be properly initialized")
	Node() : isRoot(true) {}
	void setinList() {isRoot = false;}
#endif

	Label getLabel(const int &labelid) const {return labels[labelid];}
	int getPost(const int &labelid) const {return getLabel(labelid).post;}
	int getPre(const int &labelid) const {return getLabel(labelid).pre;}
	void setLabel(const int &pre, const int &post, const int &id) {labels[id] = Label(pre, post);}
};

/**
 * @brief GRA is used as an alias for a vector or vertexes 
 */
typedef std::vector<Node> Gra;

/**
 * @brief Class Graph
 * Contains a vector of vertexes and a number of labelings 
 */
class Graph {
	private:
		Gra graph;
	public:
		Graph();
		Graph(Gra&);
		Graph(const std::string&, ThreadPool&);
		~Graph();

		void makeinList(Gra &);

		EdgeList getRoots() const;
		EdgeList& out_edges(const int);
#if BIDI
		EdgeList& in_edges(const int);
#endif

		int out_degree(const int);
#if BIDI
		int in_degree(const int);
#endif
		int num_edges() const;
		int num_vertices() const;
		
		void printGraph();
		void writeGraph(std::ostream&);

		Graph& operator=(const Graph&);
		Node& operator[](const int&);

		void clear();
};

void readChunk(const std::string&, Gra &, const int, const int);

#endif /* GRAPH_H_ */
