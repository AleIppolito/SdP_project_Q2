
#ifndef GRAPH_H_
#define GRAPH_H_

#include "Threadpool.h"


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
		Graph(const std::string&);
		~Graph();

		void readGraph(const std::string&);
		void addEdge(const int&, const int&);
		void addVertex(const int&);

		
		bool contains(const int&, const int&, const int&) const;
		void writeGraph(std::ostream&);
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

#endif /* GRAPH_H_ */
