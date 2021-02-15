/*
 * @file Graph.h
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * 
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
	EdgeList inList;
	Node(const EdgeList in, const EdgeList out) : inList(in), outList(out) {} // @suppress("Class members should be properly initialized")
	Node(){}

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
		void readChunk(const std::string&, const int, const int);
		void makeinList(Gra &);

		EdgeList getRoots() const;
		EdgeList& outEdges(const int);
		EdgeList& inEdges(const int);

		int outDegree(const int);
		int inDegree(const int);
		int numEdges() const;
		int numVertices() const;
		
		void writeGraph(std::ostream&);

		Graph& operator=(const Graph&);
		Node& operator[](const int&);

		void clear();
};



#endif /* GRAPH_H_ */
