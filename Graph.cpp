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

Graph::Graph() {graph = GRA();} // @suppress("Class members should be properly initialized")

Graph::Graph(GRA& g) {graph = g;} // @suppress("Class members should be properly initialized")

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
	int size, start_line = 1;		// skip first line = size
	in >> size;
	in.close();
	graph = GRA(size, Node());
	int chunk_size = size/CHUNK_N;
	do {
		p.addJob(readChunk, std::ref(filename), std::ref(graph), start_line, start_line+chunk_size);
		start_line += chunk_size +1;		// don't read twice
		chunk_size = (start_line+chunk_size < size) ? chunk_size : size-start_line;
	} while (start_line < size);
	p.waitFinished();
	makeinList(graph);
}

Graph::~Graph() {}

void readChunk(const std::string &file, GRA &gr, const int start, const int end) {
	ifstream fs(file);
	if (!fs) {
		cout << "Error: Cannot open " << file << endl;
		exit(EXIT_FAILURE);
	}
	int sid = 0, tid = 0, i = 0;
	char hash;
	while(i++ < start)
		fs.ignore(UINT_MAX, '\n');
	while(fs >> sid >> hash >> std::ws && sid < end) {
		while(fs.peek() != '#' && fs >> tid >> std::ws)
			gr[sid].outList.push_back(tid);
		fs.ignore();
	}
}

void Graph::makeinList(GRA &gra) {
	for(int i=0; i<gra.size(); i++)
		for(int &tg : gra[i].outList)
			gra[tg].inList.push_back(i);
}

/**
 * @brief Graph clear removes the node vector
 * 
 */
void Graph::clear() {graph.clear();}

/*
/**
 * @brief Read the graph from a file path. This file need a certain structure:
 * n <-- number of vertexes
 * src0 : trg1 trg2 trg3 ... #
 * src1 : trg1 trg2 trg3 ... #
 * Use the ifstream operator >> to read form the file in a formatted output
 * @param file 
 *
void Graph::readGraph(const std::string &file) {
	ifstream in(file, std::ios::binary);
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

/**
 * @brief Add an edge (after correctness checks) to a src vertex by pushing the target vertex 
 * to its outlist and the src vertex to the target inlist 
 * 
 * @param src 
 * @param trg 
 *
void Graph::addEdge(const int &src, const int &trg) {
	if (src >= graph.size())
		addVertex(src);
	if (trg >= graph.size())
		addVertex(trg);
	graph[trg].inList.push_back(src);
	graph[src].outList.push_back(trg);
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
 * @brief Containment check for labeling purposes
 *  * 
 * @param src 
 * @param trg 
 * @param dim 
 * @return true 
 * @return false 
 */
bool Graph::contains(const int &src, const int &trg, const int &dim) const {
	for(int i=0; i<dim; i++) {
		if(graph[src].getPre(i) > graph[trg].getPre(i))
			return false;
		if(graph[src].getPost(i) < graph[trg].getPost(i))
			return false;
	}
	return true;
}

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



std::vector<int> Graph::getRoots() const {
	std::vector<int> roots;
	int i = 0;
	for(const Node &git : graph) {
		if (git.inList.size() == 0)
			roots.push_back(i);
		i++;
	}
	return roots;
}

bool Graph::hasEdge(int &src, int &trg) {
	for (int &ei : graph[src].outList)
		if (ei == trg)
			return true;
	return false;
}

/**
 * @brief Operator override for Node list initialization
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

EdgeList& Graph::out_edges(const int &src) {return graph[src].outList;}

EdgeList& Graph::in_edges(const int &trg) {return graph[trg].inList;}

int Graph::out_degree(const int &src) {return graph[src].outList.size();}

int Graph::in_degree(const int &trg) {return graph[trg].inList.size();}

void Graph::printGraph() {writeGraph(cout);}

int Graph::num_vertices() const {return graph.size();}
