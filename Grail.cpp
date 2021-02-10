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

Grail::Grail(Graph& graph, int Dim ,ThreadPool& pool): g(graph), dim(Dim) { // @suppress("Class members should be properly initialized")
	int i;
	for(i = 0; i <  g.num_vertices(); i++)
		graph[i].labels.resize(dim);
	for(i=0; i<dim; i++)
		pool.addJob(randomlabeling, std::ref(graph), i);
	pool.waitFinished();
}

Grail::~Grail() {}

/******************************************************************
 * Labeling Functions
 ****************************************************************/

// compute interval label for each node of tree (pre_order, post_order)
void Grail::randomlabeling(Graph& tree, unsigned short int labelid) {
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
	std::vector<int> roots = tree.getRoots();
	std::vector<int>::iterator sit;
	int pre_post = 0;
	std::vector<bool> visited(tree.num_vertices(), false);

	std::shuffle(roots.begin(), roots.end(),gen);

	for (sit = roots.begin(); sit != roots.end(); sit++)
		visit(tree, *sit, ++pre_post, visited, labelid, gen);
}	

// traverse tree to label node with pre and post order by giving a start node
int Grail::visit(Graph& tree, int vid, int& pre_post, std::vector<bool>& visited, unsigned short int labelid, std::mt19937& gen ) {
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);

	std::shuffle(el.begin(), el.end(),gen);

	EdgeList::iterator eit;
	int pre_order = tree.num_vertices()+1;
	for(eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit])
			pre_order = std::min(pre_order, visit(tree, *eit, pre_post, visited, labelid, gen));
		else
			pre_order = std::min(pre_order, tree[*eit].getPre(labelid) );
	}
	pre_order = std::min(pre_order, pre_post);
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

char Grail::bidirectionalReach(int src,int trg, int query_id, std::vector<int>& visited){
	/*Check trivial cases first: 
	* src == trg reachable 
	* src has no children or trg has no parents then reachability is impossible
	* src does not contain trg then it's not reachable
	*/

	if(src == trg ) return 'r';
	if( !contains(src,trg)) return 'n';
	else if(!g.out_degree(src) || !g.in_degree(trg)) return 'f';

	query_id++;					// 0 is = -0
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
			if(visited[*ei]==-query_id) return 'r';
			else if(visited[*ei]!=query_id && contains(*ei,trg)) {
				forward.push(*ei);
				visited[*ei] = query_id;
			}
		//LOOK UP
		next = backward.front();
		backward.pop();
		el = g.in_edges(next);
		for (ei = el.begin(); ei != el.end(); ei++)
			if(visited[*ei]==query_id) return 'r';
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

 
/**
 * @brief Printing function for debugging purposes, prints the labeling for each node
 * 
 * @param out 
 * @param g 
 * @param dim 
 */
void print_labeling(std::ostream &out, Graph &g, int dim){
	for(int i = 0; i < dim; i++){
		out << "Printing labeling " << i << endl;
		for(int j = 0; j < g.num_vertices();j++){
			out << j << " " << g[j].getPre(i) << " " << g[j].getPost(i) << endl; 
		}
	}
}
