/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef GRAIL_H_
#define GRAIL_H_

#include "Graph.h"
		
class Grail {
	private:
		Graph& g;
		int dim;
		std::vector<char> reachability;
	public:
#if THREADS
		Grail(Graph& graph, int dim, ThreadPool&);
#else
		Grail(Graph& graph, int dim);
#endif
		~Grail();
		static int visit(Graph& tree, int vid, int& pre_post, std::vector<bool>& visited, unsigned short int labelid);
		static void randomlabeling(Graph& tree, unsigned short int labelid);
		Graph& getGraph(){ return g;	};
		char getReachability(int n) {return reachability[n];}
		void setReach(const int &query_id, const char &sol) {reachability[query_id] = sol;}
		void setReachabilty(const int &n) {reachability.resize(n);};
		char bidirectionalReach(int src, int trg, int query_id, std::vector<int>& visited);
		bool contains(const int &, const int&);
		//void index_size(int* ind_size);
};

void print_labeling(std::ostream &out, Graph &g, int dim);

#endif /* GRAIL_H_ */
