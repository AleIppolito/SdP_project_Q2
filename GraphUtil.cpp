/* Copyright (c) Hilmi Yildirim 2010,2011.

The software is provided on an as is basis for research purposes.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.
*/
#include "GraphUtil.h"
// depth first search given a start node
void GraphUtil::dfs(Graph& g, int vid, vector<int>& preorder, vector<int>& postorder, vector<bool>& visited) {
	visited[vid] = true;
	preorder.push_back(vid);
	EdgeList el = g.out_edges(vid);
	EdgeList::iterator eit;
	int nextid = -1;
	// check whether all child nodes are visited
	for (eit = el.begin(); eit != el.end(); eit++) {
		if (!visited[*eit]) {
			nextid = *eit;
			dfs(g, nextid, preorder, postorder, visited);
		}
	}
	postorder.push_back(vid);
}

// topological sorting by depth-first search
// return reverse order of topological_sorting list
void GraphUtil::topological_sort(Graph g, vector<int>& ts) {
	vector<bool> visited(g.num_vertices(), false);
	vector<int> preorder;
	vector<int> postorder;
	vector<int> roots = g.getRoots();
	vector<int>::iterator sit;
	// depth-first-search whole graph
	for (sit = roots.begin(); sit != roots.end(); sit++) 
		if (!visited[*sit])
			dfs(g, (*sit), preorder, postorder, visited);
	
	ts = postorder;

	// for test
/*
	cout << "Topo sorting(GraphUtil): ";
	vector<int>::iterator vit;
	for (vit = ts.begin(); vit != ts.end(); vit++)
		cout << (*vit) << " ";
	cout << endl;
*/
}


// traverse tree to label node with pre and post order by giving a start node
// using GRIPP's labeling method
void GraphUtil::traverse(Graph& tree, int vid, int& pre_post, vector<bool>& visited) {
	visited[vid] = true;
	EdgeList el = tree.out_edges(vid);
	EdgeList::iterator eit;
	int pre_order;
	for (eit = el.begin(); eit != el.end(); eit++) {
		pre_order = pre_post;
		pre_post++;
		if (!visited[*eit])
			traverse(tree, *eit, pre_post, visited);
		tree[*eit].pre_order = pre_order;
		tree[*eit].post_order = pre_post;
		pre_post++;
	}
}

// compute interval label for each node of tree (pre_order, post_order)
void GraphUtil::pre_post_labeling(Graph& tree) {
	vector<int> roots = tree.getRoots();
	vector<int>::iterator sit;
	int pre_post = 0;
	int pre_order = 0;
	vector<bool> visited(tree.num_vertices(), false);
	
	for (sit = roots.begin(); sit != roots.end(); sit++) {
		pre_order = pre_post;
		pre_post++;
		traverse(tree, *sit, pre_post, visited);
		tree[*sit].pre_order = pre_order;
		tree[*sit].post_order = pre_post;
		pre_post++;
	}
}
