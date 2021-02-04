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
std::mutex label_lock;

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

Grail::Grail(Graph& graph, int Dim, ThreadPool& p): g(graph), dim(Dim) { // @suppress("Class members should be properly initialized")
	int8_t i;
	int maxid = g.num_vertices();
	
	for(i = 0 ; i<maxid; i++){
		graph[i].labels.resize(dim);
		//graph[i].middle = new int[dim];
	}

	int maxThread = std::thread::hardware_concurrency();
	int nThreads = min(dim, maxThread);
	for(i=0; i<dim; i++){
#if THREADS

		//threadPool.emplace_back(std::thread(&randomlabeling, ref(graph), i));
#else
		randomlabeling(graph,i);
#endif
		cout << "Labeling " << i << " in progress" << endl;
	}
#if THREADS
	/*for(auto &t : threadPool)
		t.join();*/
#endif

	cout << "labelings completed" << endl;
	// PositiveCut = NegativeCut = TotalCall = TotalDepth = CurrentDepth = 0;
}

Grail::~Grail() {
}


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
	for (sit = roots.begin(); sit != roots.end(); sit++) {
		pre_post++;
		visit(tree, *sit, pre_post, visited, labelid);
	}
}


// traverse tree to label node with pre and post order by giving a start node
int Grail::visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited,
		 		unsigned short int labelid) {
 	//cout << "entering " << vid << "labelid" << endl;
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	random_shuffle(el.begin(),el.end());
	EdgeList::iterator eit;
	int pre_order = tree.num_vertices()+1;
	//tree[vid].middle->push_back(pre_post);
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]){
			pre_order = min(pre_order, visit(tree, *eit, pre_post, visited, labelid));
		} else {
			pre_order = min(pre_order, tree[*eit].getPre(labelid) );
		}
	}
	pre_order = min(pre_order, pre_post);
	tree[vid].labels.at(labelid) = Label(pre_order,pre_post);
	cout << "Copying vertes label " << vid << " " << tree[vid].getPre(labelid) << " " << tree[vid].getPost(labelid) << endl;
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
	for(int i=0;i<dim;i++){
			if(g[src].getPre(i) > g[trg].getPre(i)) {
				return false;
			}
			if(g[src].getPost(i) < g[trg].getPost(i)){
				return false;
			}
		}
	return true;
}

bool Grail::bidirectionalReach(int src,int trg){
	/*Check trivial cases first: 
	* src == trg reachable 
	* src has no children or trg has no parents then reachability is impossible
	* src does not contain trg then it's not reachable
	*/
	if(src == trg )
		return true;
	if( !g.out_degree(src) || !g.in_degree(trg) || !contains(src,trg))
		return false;
	

	std::queue<int> forward;
	std::queue<int> backward;
	std::vector<char> curvisit( g.num_vertices(), 'n');

	curvisit[src] = 'f';
	forward.push(src);
	curvisit[trg] = 'b';
	backward.push(trg);

	EdgeList el;
	std::vector<int>::iterator ei;
	char next;
	while(!forward.empty() && !backward.empty()){
		//LOOK DOWN
		next = forward.front();
		forward.pop();
		el = g.out_edges(next);
		//for each child of start node
			for (ei = el.begin(); ei != el.end(); ei++){
				if(curvisit[*ei]=='b'){
					return true;
				}else if(curvisit[*ei]!='f' && contains( *ei,trg ) ){
					forward.push(*ei);
					curvisit[*ei] = 'f';
				}
			}
		//LOOK UP
		next = backward.front();
		backward.pop();
		el = g.in_edges(next);

			for (ei = el.begin(); ei != el.end(); ei++){
				if(curvisit[*ei]=='f'){
					return true;
				}else if(curvisit[*ei]!='b' && contains(src,*ei) ){
					backward.push(*ei);
					curvisit[*ei]='b';
				}
			}

	}
	return false;
}

