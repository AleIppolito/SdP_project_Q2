/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Grail.h"

/*
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
};*/

Grail::Grail(Graph& graph, int Dim 
#if THREADS
	,ThreadPool& pool
#endif
	): g(graph), dim(Dim) { // @suppress("Class members should be properly initialized")
	int i, maxid = g.num_vertices();
	for(i = 0; i < maxid; i++)
		graph[i].labels.resize(dim);
	for(i=0; i<dim; i++)
#if THREADS
		pool.addJob(randomlabeling, std::ref(graph), i);
#else
		randomlabeling(graph,i);
#endif
#if THREADS
	pool.waitFinished();
#endif
}

Grail::~Grail() {}

/******************************************************************
 * Labeling Functions
 ****************************************************************/

// compute interval label for each node of tree (pre_order, post_order)
void Grail::randomlabeling(Graph& tree, unsigned short int labelid) {
	std::vector<int> roots = tree.getRoots();
	std::vector<int>::iterator sit;
	int pre_post = 0;
	std::vector<bool> visited(tree.num_vertices(), false);
	random_shuffle(roots.begin(),roots.end());
	for (sit = roots.begin(); sit != roots.end(); sit++)
		visit(tree, *sit, ++pre_post, visited, labelid);
	/*
	for (sit = roots.begin(); sit != roots.end(); sit++) {
		pre_post++;
		visit(tree, *sit, pre_post, visited, labelid);
	}
	 */
}	

// traverse tree to label node with pre and post order by giving a start node
int Grail::visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, unsigned short int labelid) {
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	random_shuffle(el.begin(),el.end());
	EdgeList::iterator eit;
	int pre_order = tree.num_vertices()+1;
	for(eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit])
			pre_order = min(pre_order, visit(tree, *eit, pre_post, visited, labelid));
		else
			pre_order = min(pre_order, tree[*eit].getPre(labelid) );
	}
	pre_order = min(pre_order, pre_post);
	tree[vid].setLabel(pre_order, pre_post, labelid);
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
bool Grail::contains(const int &src, const int &trg) {
	for(int i=0;i<dim;i++) {
		if(g[src].getPre(i) > g[trg].getPre(i))
			return false;
		if(g[src].getPost(i) < g[trg].getPost(i))
			return false;
	}
	return true;
}

void Grail::bidirectionalReach(int src,int trg, int query_id){
	/*Check trivial cases first: 
	* src == trg reachable 
	* src has no children or trg has no parents then reachability is impossible
	* src does not contain trg then it's not reachable
	*/
	if(src == trg ) {
		reachability[query_id] = 'r'; 
		return;
	}

	if(!g.out_degree(src) |! g.in_degree(trg) |! contains(src,trg)) {
		reachability[query_id] = 'n';
		return;
	}
	
	std::queue<int> forward;
	std::queue<int> backward;
	std::vector<char> curvisit(g.num_vertices(), 'x');

	curvisit[src] = 'f';
	forward.push(src);
	curvisit[trg] = 'b';
	backward.push(trg);

	EdgeList el;
	std::vector<int>::iterator ei;
	int next;

	while(!forward.empty() &! backward.empty()) {
		//LOOK DOWN
		next = forward.front();
		forward.pop();
		el = g.out_edges(next);
		//for each child of start node
		for (ei = el.begin(); ei != el.end(); ei++) {
			if(curvisit[*ei]=='b') {
				reachability[query_id] = 'r';
				return;
			} else if(curvisit[*ei]!='f' && contains(*ei,trg)) {
				forward.push(*ei);
				curvisit[*ei] = 'f';
			}
		}
		//LOOK UP
		next = backward.front();
		backward.pop();
		el = g.in_edges(next);
		for (ei = el.begin(); ei != el.end(); ei++) {
			if(curvisit[*ei]=='f') {
				reachability[query_id] = 'r';
				return;
			} else if(curvisit[*ei]!='b' && contains(src,*ei)) {
				backward.push(*ei);
				curvisit[*ei]='b';
			}
		}
	}
	reachability[query_id] = 'f';
	return;
}
