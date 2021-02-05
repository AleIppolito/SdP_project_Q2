/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _BOX_H
#define _BOX_H

#include "Graph.h"

		
class Grail {
	private:
		Graph& g;
		int dim;
		std::vector<char> reachability;
	public:
		// unsigned int PositiveCut, NegativeCut, TotalCall, TotalDepth, CurrentDepth;
		
		#if THREADS
		Grail(Graph& graph, int dim, ThreadPool&);
		#else
		Grail(Graph& graph, int dim);
		#endif
		~Grail();
		static int visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, unsigned short int labelid);
		static void randomlabeling(Graph& tree, unsigned short int labelid);

		char getReachability(int n){
			return reachability[n];
		}
		void setReach(int query_id, char sol){
			reachability[query_id] = sol;
		}
		void setReachabilty(int n);
		void printLabeling(Graph& tree, int i, ostream& out);
		void bidirectionalReach(int src, int trg, int query_id);
		bool contains(int src, int trg);
		void index_size(int* ind_size);
};


#endif


