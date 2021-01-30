/* Copyright (c) Hilmi Yildirim 2010,2011.

The software is provided on an as is basis for research purposes.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.
*/
#include "Grail.h"
#include <thread>
#include <string>
//#include "TCSEstimator.h"
#include <queue>

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


/*******************************************************************************************
GRAIL LABELING :
		1- Constructor:
		2- SetIndex - an auxillary function that is used by fixed reverse pairs
		3- fixedreverselabeling - labels with fixed reverse random ordering
		4- randomlabeling - random ordering
		6- fixedreversevisit - used by fixedreverselabeling
		7- visit - used by random labeling
*******************************************************************************************/

Grail::Grail(Graph& graph, int Dim): g(graph),dim(Dim) {
	int i,maxid = g.num_vertices();
	visited = new int[maxid];
	QueryCnt = 0;
	for(i = 0 ; i< maxid; i++){
		//TODO destroy element after grail program
		graph[i].pre = new vector<int>(dim);
		graph[i].post = new vector<int>(dim);
		graph[i].middle = new vector<int>(dim);
		visited[i]=-1;
	}
	cout << "Graph Size = " << maxid << endl;
	vector<std::thread> threadPool;
	//int maxThread = thread::hardware_concurrency();
	for(i=0;i<dim;i++){
		threadPool.push_back(thread(&randomlabeling,ref(graph),ref(i)));
		//randomlabeling(graph,i);
		cout << "Labeling " << i << " is completed" << endl;
	}
	for(i=0;i<dim;i++){
			threadPool.at(i).join();
		}
	PositiveCut = NegativeCut = TotalCall = TotalDepth = CurrentDepth = 0;
}

Grail::~Grail() {
}


/******************************************************************
 * Labeling Functions
 ****************************************************************/


// compute interval label for each node of tree (pre_order, post_order)
void Grail::randomlabeling(Graph& tree, int labelid) {

	vector<int> roots = tree.getRoots();
	vector<int>::iterator sit;
	int pre_post = 0;
	vector<bool> visited(tree.num_vertices(), false);
	//random_shuffle(roots.begin(),roots.end());
	for (sit = roots.begin(); sit != roots.end(); sit++) {
		pre_post++;
		visit(tree, *sit, pre_post, visited, labelid);
	}
}


void Grail::printLabeling(Graph& tree, int i, ostream& out){

	for(int j = 0; j < tree.num_vertices(); j++){
		out <<	"Node : " << j << " Pre : " << tree[j].pre->at(i) << " Post : " <<	tree[j].post->at(i) << endl;

	}
}

// traverse tree to label node with pre and post order by giving a start node
int Grail::visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, int labelid) {
//	cout << "entering " << vid << endl;
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	//random_shuffle(el.begin(),el.end());
	EdgeList::iterator eit;
	int pre_order = tree.num_vertices()+1;
	tree[vid].middle->push_back(pre_post);
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]){
			pre_order=min(pre_order,visit(tree, *eit, pre_post, visited, labelid));
		}else{
			pre_order=min(pre_order,tree[*eit].pre->at(labelid));
			//This preorder check assumes that the last label added to the pre vector
			//happened during current traversal
		}
	}
	pre_order=min(pre_order,pre_post);
	tree[vid].pre->at(labelid)=pre_order;
	tree[vid].post->at(labelid)=pre_post;
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
	queue<int> forward;
	queue<int> backward;
	if(src == trg )
		return true;

	if(!contains(src,trg))						// if it does not contain reject
		return false;
	
	QueryCnt++;
	visited[src] = QueryCnt;
	forward.push(src);
	visited[trg] = -QueryCnt;
	backward.push(trg);

	EdgeList el;
	vector<int>::iterator ei;
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

