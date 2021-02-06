/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Graph.h"

Graph::Graph() {
	graph = GRA();
	vsize = 0;
}

Graph::Graph(GRA& g) {
	vsize = g.size();
	graph = g;
}


#if THREADS
Graph::Graph(char* filename,ThreadPool& p) {
		readGraph(filename);
		//readGraph(filename,p);
	}
#else
	Graph::Graph(char *filename) {
		readGraph(filename);
	}
#endif

Graph::~Graph() {}

void Graph::printGraph() {
	writeGraph(cout);
}

void Graph::clear() {
	vsize = 0;
	graph.clear();
}

bool Graph::contains(int src,int trg,int dim){
	int i;
	for(i=0; i<dim; i++){ 
			if(graph[src].vertex.getPre(i) > graph[trg].vertex.getPre(i))
				return false;
			if(graph[src].vertex.getPost(i) < graph[trg].vertex.getPost(i))
				return false;
		}
	return true;
}

bool Graph::incrementalContains(int src,int trg,int cur){
	int i;
	for(i=0; i<cur; i++){
			if(graph[src].vertex.getPre(i) > graph[trg].vertex.getPre(i))
				return false;
			if(graph[src].vertex.getPost(i) < graph[trg].vertex.getPost(i))
				return false;
		}
	return true;
}
void Graph::readGraph(char *filename) {
	ifstream in(filename);
	if (!in) {
		cout << "Error: Cannot open " << filename << endl;
		return ;
	}
	int n;
	in >> n;
	// initialize
	vsize = n;
	graph = GRA(n, Node());
	int sid = 0;
	int tid = 0;
	char hash;
	while (in >> sid >> hash >> std::ws) {
		while (in.peek() != '#' && in >> tid >> std::ws) {
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > n)
				cout << "Wrong tid " << tid << endl;
			addEdge(sid,tid);
		}
		in.ignore();
	}
	
	
}

void Graph::writeGraph(ostream& out) {
	cout << "Graph size = " << graph.size() << endl;
	out << "graph_for_greach" << endl;
	out << graph.size() << endl;

	GRA::iterator git;
	EdgeList el;
	EdgeList::iterator eit;
	for (int i = 0; i < graph.size(); i++) {
		out << i << ": ";
		el = graph[i].outList;
		for (eit = el.begin(); eit != el.end(); eit++)
			out << (*eit) << " ";
		out << "#" << endl;
	}
}

void Graph::addVertex(int vid) {
	if (vid >= graph.size()) {
		int size = graph.size();
		for (int i = 0; i < (vid-size+1); i++) {
			graph.push_back(Node(vid+i));
		}
		vsize = graph.size();
	}

	Vertex v;
	v.id = vid;
	cout << "Entering vertex : " << v.id << endl;
	v.top_level = -1;
	graph[vid] = Node(v,EdgeList(),EdgeList());
}

void Graph::addEdge(int sid, int tid) {
		
	if (sid >= graph.size()){
		addVertex(sid);}
	if (tid >= graph.size()){
		addVertex(tid);}
	// update edge list
	
	graph[sid].outList.push_back(tid);
	graph[tid].inList.push_back(sid);
}


int Graph::num_vertices() {
	return vsize;
}

int Graph::num_edges() {
	EdgeList el;
	GRA::iterator git;
	int num = 0;
	for (git = graph.begin(); git != graph.end(); git++) {
		el = git->outList;
		num += el.size();
	}
	return num;
}

// return out edges of specified vertex
EdgeList& Graph::out_edges(int src) {
	return graph[src].outList;
}

// return in edges of specified vertex
EdgeList& Graph::in_edges(int trg) {
	return graph[trg].inList;
}

int Graph::out_degree(int src) {
	return graph[src].outList.size();
}

int Graph::in_degree(int trg) {
	return graph[trg].inList.size();
}

// get roots of graph (root is zero in_degree vertex)
vector<int> Graph::getRoots() {
	vector<int> roots;
	GRA::iterator git;
	int i = 0;
	for (git = graph.begin(), i = 0; git != graph.end(); git++, i++) {
		if (git->inList.size() == 0)
			roots.push_back(i);
	}
	
	return roots;
}

// check whether the edge from src to trg is in the graph
bool Graph::hasEdge(int src, int trg) {
	EdgeList el = graph[src].outList;
	EdgeList::iterator ei;
	for (ei = el.begin(); ei != el.end(); ei++)
		if ((*ei) == trg)
			return true;
	return false;

}

// return vertex list of graph
GRA& Graph::vertexes() {
	return this->graph;
}

Graph& Graph::operator=(const Graph& g) {
	if (this != &g) {
		graph = g.graph;
		vsize = g.vsize;
	}
	return *this;
}

// get a specified vertex property
Vertex& Graph::operator[](const int vid) {
	return graph[vid].vertex;
}
