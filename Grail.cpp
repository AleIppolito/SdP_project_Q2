/*
 * @file Grail.cpp
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 *
 */

#include "Grail.h"

/**
 * @brief Construct a new Grail:: Grail object, each node has a vector of labels which
 * are now resized to meet the required traversal number, then each labeling is given
 * as a task to the threadpool and the main thread waits.
 * 
 * @param graph 
 * @param Dim 
 * @param testfilename
 * @param pool 
 */
Grail::Grail(Graph& graph, const int Dim ,const std::string testfilename, ThreadPool& pool): g(graph), dim(Dim) {
	int i;
	pool.addJob( [this,testfilename] { readQueries(testfilename) ;} );
	for(i=0; i<g.numVertices(); i++)
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
 * @param labelid 
 */
void Grail::randomlabeling(const unsigned short labelid) {
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
	EdgeList roots = g.getRoots();
	EdgeList::iterator sit;
	int pre_post = 0;
	std::vector<bool> visited(g.numVertices(), false);
	std::shuffle(roots.begin(), roots.end(), gen);
	for (sit = roots.begin(); sit != roots.end(); sit++)
		visit(*sit, ++pre_post, visited, labelid, gen);
}	

/**
 * @brief Basic visit, the tree is visited from vid randomly in a post order
 * traversal
 * 
 * @param vid 
 * @param pre_post 
 * @param visited 
 * @param labelid 
 * @param gen 
 * @return int 
 */
int Grail::visit(const int vid, int& pre_post, std::vector<bool>& visited, const unsigned short labelid, std::mt19937 &gen) {
	visited[vid] = true;
	EdgeList el = g.outEdges(vid);
	std::shuffle(el.begin(), el.end(), gen);
	EdgeList::iterator eit;
	int pre_order = g.numVertices() + 1;

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
	if( !g.outDegree(src) || !g.inDegree(trg) )
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
		el = g.outEdges(next);

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
		el = g.inEdges(next);
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

/*************************************************************************************
Helper functions 
*************************************************************************************/

int Grail::getQuerySize() const {
	return queries.size();
}
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
void Grail::groundTruthCheck(std::ostream& out){
	int i = 0, success = 0, fail = 0;
	for(int i =0; i<queries.size(); i++)
		if( (!queries[i].labels && (reachability[i] == 'f' || reachability[i] == 'n' ))  || ( queries[i].labels && reachability[i] == 'r') )
			success++;
		else fail++;
	out.precision(2);
	out << "Success rate: " << success/(success+fail)*100 << "%" << endl;
}
#endif

#if DEBUG
/**
 * @brief Printing function for debugging purposes, prints the labeling for each node
 * 
 * @param out 
 * @param g 
 * @param dim 
 */
void Grail::printLabeling(std::ostream &out) {
	for(int i=0; i<dim; i++){
		for(int j = 0; j < g.numVertices();j++){
			out << j << " " << g[j].getPre(i) << " " << g[j].getPost(i) << endl; 
		}
	}
}


void Grail::printQuery(std::ostream &out) {
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
void Grail::printReach(std::ostream &out) {
	for (int i=0; i<queries.size(); i++)
		out << queries[i].src << " " << queries[i].trg << " " << reachability[i] << endl;
}


#endif
/**
 * @brief This function reads TEST_FILENAME and saves the queries in a std::vector of queries, a struct
 * that contains 2 integers (src,trg)
 * File structure required:
 *  src  trg opt(label)
 * 	int  int 
 * 	int  int 
 *  ...  ...
 * @param tfname TEST_FILENAME
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
			while(fstr >> src >> trg )
				queries.push_back({src, trg});
	#endif
}


/**
 * @brief READS THE GRAIL AND STORES IT INTO THE GRAIL OBJECT 
 * Answer all reachability queries from start to end inside queries array
 * @param start
 * @param end
 * */
void Grail::reachWrapper(int start, int end) {
	std::vector<int> visited(g.numVertices());
	for (int i=start; i<end; i++)
		reachability[i] = bidirectionalReach(queries[i].src, queries[i].trg, i, visited);
}
