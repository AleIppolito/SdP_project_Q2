/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _GRAIL_H
#define _GRAIL_H

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
		static int visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, unsigned short int labelid);
		static void randomlabeling(Graph& tree, unsigned short int labelid);

		char getReachability(const int &n) {return reachability[n];}
		void setReach(const int &query_id, const char &sol) {reachability[query_id] = sol;}
		void setReachabilty(const int &n) {reachability.resize(n);};
		void bidirectionalReach(int src, int trg, int query_id);
		bool contains(const int &, const int&);
		//void printLabeling(Graph& tree, int i, const ostream&);
		//void index_size(int* ind_size);
};

#endif


