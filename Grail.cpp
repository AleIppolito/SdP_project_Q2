/*
 * @file Grail.cpp
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @copyright Copyright (c) 2021
 */

#include "Grail.h"

/**
 * @brief Construct a new Grail:: Grail object, each node has a vector of labels which
 * are now resized to meet the required traversal number, then each labeling is given
 * as a task to the threadpool and the main thread waits.
 * 
 * @param graph 
 * @param Dim 
 * @param pool 
 */
Grail::Grail(Graph& graph, const int Dim ,const std::string testfilename, ThreadPool& pool): g(graph), dim(Dim) {
	int i;
	pool.addJob( [this,testfilename] { readQueries(testfilename) ;} );
	for(i=0; i<g.num_vertices(); i++)
		graph[i].labels.resize(dim);
	for(i=0; i<dim; i++)
		pool.addJob( [this, i] { randomlabeling(i); } );
	pool.waitFinished();
}

Grail::~Grail() {}

/******************************************************************
 * Labeling Functions
 ****************************************************************/

/**
 * @brief Wrapper function that starts a post order visit from each root, each root is
 * chosen randomly.
 * 
 * @param tree 
 * @param labelid 
 */
void Grail::randomlabeling(const unsigned short labelid) {
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
	EdgeList roots = g.getRoots();
	EdgeList::iterator sit;
	int pre_post = 0;
	std::vector<bool> visited(g.num_vertices(), false);
	std::shuffle(roots.begin(), roots.end(), gen);
	for (sit = roots.begin(); sit != roots.end(); sit++)
		visit(*sit, ++pre_post, visited, labelid, gen);
}	

/**
 * @brief Basic visit, the tree is visited from vid randomly in a post order
 * traversal
 * 
 * @param tree 
 * @param vid 
 * @param pre_post 
 * @param visited 
 * @param labelid 
 * @param gen 
 * @return int 
 */
int Grail::visit(const int vid, int& pre_post, std::vector<bool>& visited, const unsigned short labelid, std::mt19937 &gen) {
	visited[vid] = true;
	EdgeList el = g.out_edges(vid);
	std::shuffle(el.begin(), el.end(), gen);
	EdgeList::iterator eit;
	int pre_order = g.num_vertices() + 1;

	for(eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit])
			pre_order = std::min(pre_order, visit(*eit, pre_post, visited, labelid, gen));
		else
			pre_order = std::min(pre_order, g[*eit].getPre(labelid) );
	}
	pre_order = std::min(pre_order, pre_post);
	g[vid].setLabel(pre_order, pre_post, labelid);
	pre_post++;
	return pre_order;
}

/*************************************************************************************
GRAIL Query Functions
*************************************************************************************/
/*
 * This function checks that src node is contained by trg node by checking each label for
 * an exception
 */
bool Grail::contains(const int src, const int trg) {
	for(int i=0;i<dim;i++) {
		if(g[src].getPre(i) > g[trg].getPre(i))
			return false;
		if(g[src].getPost(i) < g[trg].getPost(i))
			return false;

	}
	return true;
}

#if BIDI
/**
 * @brief Bidirectional Reach answers reachability queries by first checking trivial cases and then
 * exploring the graph from 2 positions, target and source. Each iteration of the while expands the forward
 * or backward frontier both of which label they path until either of 3 conditions happen
 * ->Forward frontier encounters a backwards labeled path
 * ->Backwards frontier encounters a forward labeled path
 * ->Forward/Backward frontiers has completely expanded and src/trg hasn't found trg/src (false positive)
 * 
 * @param src 
 * @param trg 
 * @param query_id 
 * @param visited 
 * @return char 
 */
char Grail::bidirectionalReach(const int src, const int trg, int query_id, std::vector<int> &visited){
	/*Check trivial cases first: 
	* src == trg reachable 
	* src has no children or trg has no parents then reachability is impossible
	* src does not contain trg then it's not reachable 
	*/

	if( src == trg )
		return 'r'; 
	if( !contains(src,trg) )
		return 'n';
	if( !g.out_degree(src) || !g.in_degree(trg) )
		return 'f';

	query_id++;					// 0 = -0
	std::queue<int> forward;
	std::queue<int> backward;
	visited[src] = query_id;
	forward.push(src);
	visited[trg] = -query_id;
	backward.push(trg);

	EdgeList el;
	EdgeList::iterator ei;
	int next;

	while(!forward.empty() &! backward.empty()) {
		//LOOK DOWN
		next = forward.front();
		forward.pop();
		el = g.out_edges(next);

		for (ei = el.begin(); ei != el.end(); ei++)
			if(visited[*ei]==-query_id){
				return 'r';
			}else if(visited[*ei]!=query_id && contains(*ei,trg)) {
				forward.push(*ei);
				visited[*ei] = query_id;
			}
		//LOOK UP
		next = backward.front();
		backward.pop();
		el = g.in_edges(next);
		for (ei = el.begin(); ei != el.end(); ei++)
			if(visited[*ei]==query_id)
				return 'r';
			else if(visited[*ei]!=-query_id && contains(src,*ei)) {
				backward.push(*ei);
				visited[*ei]=-query_id;
				}
	}
return 'f';
}
#else
/**
 * @brief Basic Reach, explores the trivial cases first and then starts a dfs that exits when src finds target or
 * src doesn't find target (false positive)
 * 
 * @param src 
 * @param trg 
 * @param query_id 
 * @param visited 
 * @return char 
 */
char Grail::reach(const int src, const int trg, int query_id, std::vector<int> &visited) {
	if(src == trg) return 'r';

	if(!contains(src,trg)) return 'n';
	if(!g.out_degree(src) || !g[trg].isRoot) return 'f';

	visited[src]=++query_id;
	return go_for_reach(src, trg, query_id, visited);
}

char Grail::go_for_reach(const int src, const int trg, int query_id, std::vector<int> &visited) {
	if(src==trg) return 'r';

	visited[src] = query_id;
	EdgeList el = g.out_edges(src);
	EdgeList::iterator eit;

	for(eit = el.begin(); eit != el.end(); eit++)
		if(visited[*eit]!=query_id && contains(*eit,trg))
			if(go_for_reach(*eit, trg, query_id, visited) == 'r') return 'r';
	return 'f';
}
#endif

/*************************************************************************************
Helper functions 
*************************************************************************************/

Graph& Grail::getGraph() const {return g;};


void Grail::setReachability(const int size){
	reachability.resize(size);
}

#if GROUND_TRUTH
/**
 * @brief If a ground truth is present then we need to check the actual result of
 * our reachability report and we use this function to find successes and fails 
 * and then print out a percentage of successes.
 * 
 * @param queries 
 */
void Grail::ground_truth_check(std::ostream& out){
	int i = 0, success = 0, fail = 0;
	for(int i =0; i<queries.size(); i++)
		if( (!queries[i].labels && (reachability[i] == 'f' || reachability[i] == 'n' ))  || ( queries[i].labels && reachability[i] == 'r') )
			success++;
		else fail++;
	out.precision(2);
	out << "Success rate: " << success/(success+fail)*100 << "%" << endl;
}
#endif


/**
 * @brief Printing function for debugging purposes, prints the labeling for each node
 * 
 * @param out 
 * @param g 
 * @param dim 
 */
void Grail::print_labeling(std::ostream &out) {
	for(int i=0; i<dim; i++){
		for(int j = 0; j < g.num_vertices();j++){
			out << j << " " << g[j].getPre(i) << " " << g[j].getPost(i) << endl; 
		}
	}
}


void Grail::print_query(std::ostream &out) {
	for(query &q: queries)
	#if GROUND_TRUTH
			out << q.src << " " << q.trg << " " << q.labels << endl;
	#else
			out << q.src << " " << q.trg << endl;
	#endif
}

/**
 * @brief Debugging function, prints the queries 
 * 
 * @param out ostream object as ouput
 * @param grail Grail object that contains the reachability result
 * @param queries queries std::vector of query struct 
 */
void Grail::print_reach(std::ostream &out) {
	for (int i=0; i<queries.size(); i++)
		out << queries[i].src << " " << queries[i].trg << " " << reachability[i] << endl;
}

/**
 * @brief This function reads TEST_FILENAME and saves the queries in a std::vector of queries, a struct
 * that contains 2 integers (src,trg)
 * File structure required:
 *  src  trg opt(label)
 * 	int  int 
 * 	int  int 
 *  ...  ...
 * @param tfname TEST_FILENAME
 * @param queries Query vector
 */
void Grail::readQueries(const std::string &testFileName) {
	int src,trg;
	ifstream fstr(testFileName);
	if (!fstr) {
		cout << "Error: Cannot open " << testFileName << endl;
		exit(EXIT_FAILURE);
	}
	#if GROUND_TRUTH
		int label;
			while(fstr >> src >> trg >> label)
				queries.push_back({src, trg, label});
	#else
			while(fstr >> src >> trg)
				queries.push_back({src, trg});
	#endif
}


/**
 * @brief READS THE GRAIL AND STORES IT INTO THE GRAIL OBJECT 
 * This function allows the threadpool to compute a non-static member function by passing its 
 * Object class as a parameter. Each thread runs end-start queries and copies them to their respective
 * vector position concurrently with other vectors, access is thread safe since each vector chunk does not 
 * overlap. Reachability is either Bidirectional or Basic Reach 
 * @param grail Grail object
 * @param src 
 * @param trg 
 * @param query_id Query number to access the std::vector of reachability results
 */
void Grail::reachWrapper(int start, int end) {
	std::vector<int> visited(g.num_vertices());
	for (int i=start; i<end; i++)
#if BIDI
		reachability[i] = bidirectionalReach(queries[i].src, queries[i].trg, i, visited);
#else
		reachability[i] = reach(queries[i].src, queries[i].trg, i, visited);
#endif
}
