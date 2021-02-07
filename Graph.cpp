/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Graph.h"

Graph::Graph() {graph = GRA();} // @suppress("Class members should be properly initialized")

Graph::Graph(GRA& g) {graph = g;} // @suppress("Class members should be properly initialized")

Graph::Graph(const string &filename) {readGraph(filename);}

Graph::~Graph() {}

void Graph::clear() {graph.clear();}

void Graph::readGraph(const string &file) {
	ifstream in(file, ios::binary);
	if (!in) {
		cout << "Error: Cannot open " << file << endl;
		return ;
	}
	int n, sid = 0, tid = 0;
	char hash;
	in >> n;
	graph = GRA(n, Node());

	while(in >> sid  >> hash >> std::ws) {
		while(in.peek() != '#' && in >> tid >> std::ws)
			addEdge(sid, tid);
		in.ignore();
	}
}

void Graph::addEdge(const int &s, const int &t) {
	if (s >= graph.size())
		addVertex(s);
	if (t >= graph.size())
		addVertex(t);
	graph[t].inList.push_back(s);
	graph[s].outList.push_back(t);
}

void Graph::addVertex(const int &vid) {
	if (vid >= graph.size()) {
		int size = graph.size();
		for (int i = 0; i < (vid-size+1); i++)
			graph.push_back(Node());
	}
	graph[vid] = Node(EdgeList(),EdgeList());
}

bool Graph::contains(const int &src, const int &trg, const int &dim) {
	for(int i=0; i<dim; i++) {
		if(graph[src].getPre(i) > graph[trg].getPre(i))
			return false;
		if(graph[src].getPost(i) < graph[trg].getPost(i))
			return false;
	}
	return true;
}

void Graph::writeGraph(ostream& out) {
	cout << "Graph size = " << graph.size() << endl;
	out << graph.size() << endl;
	for (int i = 0; i < graph.size(); i++) {
		out << i << ": ";
		for(int &el : graph[i].outList)
			out << el << " ";
		out << "#" << endl;
	}
}

int Graph::num_edges() const {
	int num = 0;
	for (const Node &git : graph)
		num += git.outList.size();
	return num;
}

// get roots of graph (root is zero in_degree vertex)
vector<int> Graph::getRoots() const {
	std::vector<int> roots;
	int i = 0;
	for(const Node &git : graph) {
		if (git.inList.size() == 0)
			roots.push_back(i);
		i++;
	}
	return roots;
}

// check whether the edge from src to trg is in the graph
bool Graph::hasEdge(int &src, int &trg) {
	for (int &ei : graph[src].outList)
		if (ei == trg)
			return true;
	return false;
}

Graph& Graph::operator=(const Graph& g) {
	if (this != &g)
		graph = g.graph;
	return *this;
}

// get a specified vertex property
Node& Graph::operator[](const int &vid) {return graph[vid];}

// return out edges of specified vertex
EdgeList& Graph::out_edges(const int &src) {return graph[src].outList;}

// return in edges of specified vertex
EdgeList& Graph::in_edges(const int &trg) {return graph[trg].inList;}

int Graph::out_degree(const int &src) {return graph[src].outList.size();}

int Graph::in_degree(const int &trg) {return graph[trg].inList.size();}

// return vertex list of graph
//GRA& Graph::vertexes() {return this->graph;}

void Graph::printGraph() {writeGraph(cout);}

int Graph::num_vertices() const {return graph.size();}
