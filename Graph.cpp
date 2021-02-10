/*
 * @file Graph.cpp
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @copyright Copyright (c) 2021
 */

#include "Graph.h"

Graph::Graph() {graph = Gra();} // @suppress("Class members should be properly initialized")

Graph::Graph(Gra& g) {graph = g;} // @suppress("Class members should be properly initialized")

/**
 * @brief Construct a new Graph:: Graph object
 * 
 * @param filename 
 * @param p ThreadPool
 */
Graph::Graph(const std::string &filename, ThreadPool&p) {
	ifstream in(filename);
	if (!in) {
		cout << "Error: Cannot open " << filename << endl;
		exit(EXIT_FAILURE);
	}

	int size, start_line = 0;		
	in >> size;
	in.close();
	graph = Gra(size, Node());
	int chunk_size = size/CHUNK_N;
	while (start_line < size) {
		p.addJob(readChunk, std::ref(filename), std::ref(graph), start_line, start_line+chunk_size);
		start_line += chunk_size;		
		chunk_size = (start_line+chunk_size < size) ? chunk_size : size-start_line;
	}
	p.waitFinished();
	makeinList(graph);
}

Graph::~Graph() {clear();}

void readChunk(const std::string &file, Gra &gr, const int start, const int end) {
	ifstream fs(file);
	if (!fs) {
		cout << "Error: Cannot open " << file << endl;
		exit(EXIT_FAILURE);
	}
	int sid = 0, tid = 0, i = 0;
	char hash;
	while(i++ <= start)
		fs.ignore(UINT_MAX, '\n');
	while(fs >> sid >> hash >> std::ws && sid < end) {
		while(fs.peek() != '#' && fs >> tid >> std::ws)
			gr[sid].outList.push_back(tid);
		fs.ignore();
	}
}

void Graph::makeinList(Gra &gra) {
	for(int i=0; i<gra.size(); i++)
		for(int &tg : gra[i].outList)
			gra[tg].inList.push_back(i);
}

/**
 * @brief Add a vertex (after correctness checks) to the graph
 * 
 * @param vid 
 *
void Graph::addVertex(const int &vid) {
	if (vid >= graph.size()) {
		int size = graph.size();
		for (int i = 0; i < (vid-size+1); i++)
			graph.push_back(Node());
	}
	graph[vid] = Node(EdgeList(),EdgeList());
}
*/

/**
 * @brief Generic graph write function, used for debugging to print out
 * the graph to an ostream
 * 
 * @param out 
 */
void Graph::writeGraph(std::ostream& out) {
	cout << "Graph size = " << graph.size() << endl;
	out << graph.size() << endl;
	for (int i = 0; i < graph.size(); i++) {
		out << i << ": ";
		for(int &el : graph[i].outList)
			out << el << " ";
		out << "#" << endl;
	}
}

EdgeList Graph::getRoots() const {
	EdgeList roots;
	int i = 0;
	for(const Node &git : graph) {
		if (git.inList.size() == 0)
			roots.push_back(i);
		i++;
	}
	return roots;
}

/**
 * @brief Operator override for copy constructor
 * 
 * @param g 
 * @return * Operator& 
 */
Graph& Graph::operator=(const Graph& g) {
	if (this != &g)
		graph = g.graph;
	return *this;
}

/**
 * @brief Operator override to access vertex properties
 * 
 * @param vid 
 * @return Node& 
 */
Node& Graph::operator[](const int &vid) {return graph[vid];}

/**
 * @brief General getter functions, used to receive info on the graph
 * Self explanatory titles.
 */

int Graph::num_edges() const {	
	int num = 0;
	for (const Node &git : graph)
		num += git.outList.size();
	return num;
}

EdgeList& Graph::out_edges(const int src) {return graph[src].outList;}

EdgeList& Graph::in_edges(const int trg) {return graph[trg].inList;}

int Graph::out_degree(const int src) {return graph[src].outList.size();}

int Graph::in_degree(const int trg) {return graph[trg].inList.size();}

void Graph::printGraph() {writeGraph(cout);}

int Graph::num_vertices() const {return graph.size();}

void Graph::clear() {graph.clear();}
