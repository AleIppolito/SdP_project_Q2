/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Grail.h"
//#include <string>
//#include <unistd.h>
//#include <mutex>
//#include "TCSEstimator.h"

vector<int> _index;
vector<double> customIndex;

template<class T> struct index_cmp {
	index_cmp(const T arr) : arr(arr) {}
	bool operator()(const size_t a, const size_t b) const
	{ return arr[a] < arr[b]; }
	const T arr;
};

template<class T> struct custom_cmp {
	custom_cmp(const T arr) : arr(arr) {}
	bool operator()(const size_t a, const size_t b) const
	{ return arr[a] > arr[b]; }
	const T arr;
};

Grail::Grail(Graph& graph, int Dim): g(graph), dim(Dim) { // @suppress("Class members should be properly initialized")
	int i,maxid = g.num_vertices();
	visited = new int[maxid];
	QueryCnt = 0;
	for(i = 0 ; i<maxid; i++){
#if VECTOR
		//TODO destroy element after grail program
		graph[i].pre = new std::vector<int>(dim);
		graph[i].post = new std::vector<int>(dim);
		graph[i].middle = new std::vector<int>(dim);
#else
		graph[i].pre = new int[dim];
		graph[i].post = new int[dim];
		graph[i].middle = new int[dim];
#endif
		visited[i] = -1;
	}
	cout << "Graph Size = " << maxid << endl;

	int maxThread = std::thread::hardware_concurrency();
	int nThreads = min(dim, maxThread);
	std::vector<std::thread> threadPool;
	for(i=0; i<dim; i++){
#if THREADS
		threadPool.emplace_back(std::thread(&randomlabeling, ref(graph), i));
#else
		randomlabeling(graph,i);
#endif
		cout << "Labeling " << i << " in progress" << endl;
	}

	for(auto &t : threadPool)
		t.join();
	cout << "labelings completed" << endl;
	// PositiveCut = NegativeCut = TotalCall = TotalDepth = CurrentDepth = 0;
}

Grail::~Grail() {
}


/******************************************************************
 * Labeling Functions
 ****************************************************************/


// compute interval label for each node of tree (pre_order, post_order)
void Grail::randomlabeling(Graph& tree, int labelid) {

	std::vector<int> roots = tree.getRoots();
	std::vector<int>::iterator sit;
	int pre_post = 0;
	std::vector<bool> visited(tree.num_vertices(), false);
	random_shuffle(roots.begin(),roots.end());
	for (sit = roots.begin(); sit != roots.end(); sit++) {
		pre_post++;
		visit(tree, *sit, pre_post, visited, labelid);
	}

}


// traverse tree to label node with pre and post order by giving a start node
int Grail::visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, int labelid) {
 	//cout << "entering " << vid << "labelid" << endl;
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	random_shuffle(el.begin(),el.end());
	EdgeList::iterator eit;
	int pre_order = tree.num_vertices()+1;
	tree[vid].middle->push_back(pre_post);
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]){
			pre_order = min(pre_order, visit(tree, *eit, pre_post, visited, labelid));
		} else {
			pre_order = min(pre_order, tree[*eit].pre->at(labelid));
		}
	}
	pre_order = min(pre_order, pre_post);
	tree[vid].pre->at(labelid) = pre_order;
	tree[vid].post->at(labelid) = pre_post;
	pre_post++;
	return pre_order;
}


/*************************************************************************************
GRAIL Query Functions
*************************************************************************************/
/*
 * This function checks that src node is contained by trg node by checking for each
 * the exception
 */
bool Grail::contains(int src,int trg){
	int i,j;
	for(i=0;i<dim;i++){
			if(g[src].pre->at(i) > g[trg].pre->at(i)) {
				return false;
			}
			if(g[src].post->at(i) < g[trg].post->at(i)){
				return false;
			}
		}
	return true;
}

bool Grail::reach(int src,int trg){
	/*
	 * Check Trivial Case first
	 */
	if(src == trg){
		return true;
	}

	if(!contains(src,trg))						// if it does not contain reject
		return false;

	visited[src]=++QueryCnt;
	return go_for_reach(src,trg);			//search for reachability in children
}

/*
 * This function traverses the tree until it finds the trg label
 * or returns false
 */
bool Grail::go_for_reach(int src, int trg) {
	if(src==trg)
		return true;
			
	visited[src] = QueryCnt;
	EdgeList el = g.out_edges(src);
	EdgeList::iterator eit;

	for (eit = el.begin(); eit != el.end(); eit++) {
		if(visited[*eit]!=QueryCnt && contains(*eit,trg)){
			if(go_for_reach(*eit,trg)){
				return true;
			}	
		}
	}
	return false;
}

bool Grail::bidirectionalReach(int src,int trg){
	std::queue<int> forward;
	std::queue<int> backward;
	if(src == trg )
		return true;

	if(!contains(src,trg))						// if it does not contain reject
		return false;
	
	QueryCnt++;

#if DEBUG

	for(int i=0; i<visited.length(); i++)
		cout << visited[i] << "\t";
	cout << endl;

#endif
	visited[src] = QueryCnt;
	forward.push(src);
	visited[trg] = -QueryCnt;
	backward.push(trg);
#if DEBUG

	for(int i=0; i<visited.length(); i++)
		cout << visited[i] << "\t";
	cout << endl;

#endif
	cout << endl;

	EdgeList el;
	std::vector<int>::iterator ei;
	int next;
	while(!forward.empty() && !backward.empty()){

		next = forward.front();
		forward.pop();
		el = g.out_edges(next);
		//for each child of start node
			for (ei = el.begin(); ei != el.end(); ei++){
				if(visited[*ei]==-QueryCnt){
					return true;
				}else if(visited[*ei]!=QueryCnt && contains( *ei,trg ) ){
					forward.push(*ei);
					visited[*ei] = QueryCnt;
				}
			}

		next = backward.front();
		backward.pop();
		el = g.in_edges(next);

			for (ei = el.begin(); ei != el.end(); ei++){
				if(visited[*ei]==QueryCnt){
					return true;
				}else if(visited[*ei]!=-QueryCnt && contains(src,*ei) ){
					backward.push(*ei);
					visited[*ei]=-QueryCnt;
				}
			}

	}
	return false;
}

