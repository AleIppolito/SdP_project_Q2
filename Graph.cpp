/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Graph.h"
std::mutex m;
Graph::Graph() {
	graph = GRA();
	vsize = 0;
}

Graph::Graph(int size) {
	vsize = size;
	graph = GRA(size, Node());
}

Graph::Graph(GRA& g) {
	vsize = g.size();
	graph = g;
}


#if THREADS
	Graph::Graph(char* filename) {
		readGraph2(filename);
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
	#if VECTOR
			if(graph[src].node.pre->at(i) > graph[src].node.pre->at(i))
	#else
			if(graph[src].node.pre[i] > graph[trg].node.pre[i])
	#endif
				return false;
	#if VECTOR
			if(graph[src].node.post->at(i) < graph[trg].node.post->at(i))
	#else
			if(graph[src].node.post[i] < graph[trg].node.post[i])
	#endif
				return false;
		}
	return true;
}

bool Graph::incrementalContains(int src,int trg,int cur){
	int i;
	for(i=0; i<cur; i++){
	#if VECTOR
			if(graph[src].node.pre->at(i) > graph[trg].node.pre->at(i))
	#else
			if(graph[src].node.pre[i] > graph[trg].node.pre[i])
	#endif
				return false;
	#if VECTOR
			if(graph[src].node.post->at(i) < graph[trg].node.post->at(i))
	#else
			if(graph[src].node.post[i] < graph[trg].node.post[i])
	#endif
				return false;
		}
	return true;
}

/*
void Graph::strTrimRight(string& str) {
	string whitespaces(" \t\r");
	int index = str.find_last_not_of(whitespaces);
	if (index != string::npos) 
		str.erase(index+1);
	else
		str.clear();
}*/
void Graph::innerRead(std::streampos start, std::streampos end, char *filename, int n){

	cout << "Start " <<start <<" "<< end<< endl;
	std::ifstream in(filename);
	if (!in) {
				cout << "Error: Cannot open " << filename << endl;
				return ;
			}

	in.seekg(start, std::ios::beg);

	int sid = 0;
	int tid=0;
	char hash;
	in >> std::ws;
	while ( in.tellg() < end && in >> sid >> hash >> std::ws) {
		while (in.peek() != '#' && in >> tid >> std::ws) {
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > n)
				cout << "Wrong tid " << tid << endl;

			m.lock();
			addEdge(sid, tid);
			m.unlock();
		}
		in.ignore();
	}
}



void Graph::readGraph2(char* filename) {
	struct timeval before_time, after_time;
	float labeling_time;
	gettimeofday(&before_time, NULL);
	int nThreads = std::thread::hardware_concurrency();
	std::vector<std::streampos> thread_pos (nThreads+1);

	std::ifstream in(filename);

	if (!in) {
		cout << "Error: Cannot open " << filename << endl;
		return ;
	}
	//get file size
	std::streampos fsize = in.tellg();
	in.seekg(0, std::ios::end);
	fsize = in.tellg() - fsize;
	in.seekg(0, std::ios::beg);

	int n;
	in >> n;

	std::streampos cur = thread_pos[0] = in.tellg();
	gettimeofday(&after_time, NULL);
	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
				(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
		cout << "#filesize setup time: " << labeling_time << " (ms)" << endl;

		gettimeofday(&before_time, NULL);
	// initialize
	vsize = n;
	graph = GRA(n, Node());

	for (int i=0; i<n; i++)
		addVertex(i);
	/*
	 * Multithread read test 1
	*/

	bool found = true;
	int c;
	int i;
	for( i = 1; i < nThreads;i++){
		cur += fsize/nThreads;
		in.seekg(cur, std::ios::beg);
		while(in.peek() != '#'){
			in.ignore();
			}
		in.ignore();
		thread_pos[i] = in.tellg();

	}
	thread_pos[nThreads] = fsize;
	std::vector<std::thread> workers;
	gettimeofday(&after_time, NULL);
		labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
					(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;

		cout << "#Chunck search time: " << labeling_time << " (ms)" << endl;
		gettimeofday(&before_time, NULL);
	for(i=0;i<nThreads;i++){
			workers.emplace_back(std::thread(&Graph::innerRead,this,thread_pos[i], thread_pos[i+1], filename,n));
		}

	for(auto &w : workers){
			w.join();
	}
	gettimeofday(&after_time, NULL);
		labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
					(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;

		cout << "#Actual work time: " << labeling_time << " (ms)" << endl;
	/*
	 * Multithread read test 2

	string buf;
	std::vector<std::thread> workers;
		int i;
		int sid = 0;
		char col;
		while (in.peek() != EOF ) {
			for( i = 0 ; i < nThreads; i++){
				cout << "entering " << i << " thread" << endl;
				sid = getSrcData(in,buf);
				workers.emplace_back(std::thread(&Graph::innerRead2,this,buf,sid,n));
				}
		}
		for(auto &w : workers){
					w.join();
		}*/
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
	cout << n << endl;
	int sid = 0;
	int tid = 0;
	char hash;
	while (in >> sid >> hash) {
		in >> std::ws;
		while (in.peek() != '#' && in >> tid >> std::ws) {
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > n)
				cout << "Wrong tid " << tid << endl;
			addEdge(sid, tid);
		}
		in.ignore();
	}
}

/*
void Graph::readGraphQ(char * filename) {

	ifstream in(filename);
	if (!in) {
		cout << "Error: Cannot open " << filename << endl;
		return ;
	}
	ThreadPool pool(3);
	pool.init();


	int n;
	in >> n;
	// initialize
	vsize = n;
	graph = GRA(n, Node());
	int sid = 0;
	int tid = 0;
	char hash;
	while (in >> sid >> hash) {
		in >> std::ws;
		while (in.peek() != '#' && in >> tid >> std::ws) {
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > n)
				cout << "Wrong tid " << tid << endl;
			pool.submit(std::bind(&Graph::addEdge,this),sid,tid);
		}
		in.ignore();
	}

	pool.shutdown();

}*/

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
	cout << "Entering node : " << v.id << endl;
	v.top_level = -1;
	graph[vid] = Node(v,EdgeList(),EdgeList());
}

void Graph::addEdge(int sid, int tid) {
	if (sid >= graph.size())
		addVertex(sid);
	if (tid >= graph.size())
		addVertex(tid);
	// update edge list
#if THREAD
	std::lock_guard<std::mutex> guard(m);
		graph[tid].inList.push_back(sid);
		graph[sid].outList.push_back(tid);
#else

		graph[tid].inList.push_back(sid);
		graph[sid].outList.push_back(tid);
#endif

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
GRA& Graph::nodes() {
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
	return graph[vid].node;
}
