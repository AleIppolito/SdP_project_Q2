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
		p.addJob( [=] {readChunk(filename, start_line, start_line+chunk_size); } );
		start_line += chunk_size;		
		chunk_size = (start_line+chunk_size < size) ? chunk_size : size-start_line;
	}
	p.waitFinished();
	makeinList(graph);
}

Graph::~Graph() {clear();}

void Graph::readChunk(const std::string &file, const int start, const int end) {
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
			graph[sid].outList.push_back(tid);
		fs.ignore();
	}
}

/**
 * @brief These functions make sure that we can find the roots in the labeling and in both reachability
 * algorithms, however reach does not require an inList to be maintained so a bool is saved to find which
 * nodes do not have ancestors (are Roots), this saves memory.
 * 
 */
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
	out << graph.size() << endl;
	for (int i = 0; i < graph.size(); i++) {
		out << i << ": ";
		for(int &el : graph[i].outList)
			out << el << " ";
		out << "#" << endl;
	}
}

/**
 * @brief Function used to start the labeling, it returns the roots as an Edgelist
 * and has 2 different implementations in case of Bidireacional (where we keep an inlist) or 
 * basic search (where we keep a boolean)
 * 
 * @return EdgeList 
 */
EdgeList Graph::getRoots() const {
	EdgeList roots;
	for(int i=0; i<graph.size(); i++)
		if(graph[i].inList.size() == 0)
			roots.push_back(i);
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

int Graph::numEdges() const {	
	int num = 0;
	for (const Node &git : graph)
		num += git.outList.size();
	return num;
}

int Graph::numVertices() const {return graph.size();}

EdgeList& Graph::inEdges(const int trg) {return graph[trg].inList;}

int Graph::inDegree(const int trg) {return graph[trg].inList.size();}


EdgeList& Graph::outEdges(const int src) {return graph[src].outList;}

int Graph::outDegree(const int src) {return graph[src].outList.size();}


void Graph::clear() {graph.clear();}
