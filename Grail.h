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
		struct timeval after_time, before_time;
		float run_time;
		int dim;
		int *visited;
		int QueryCnt;
	public:
		// unsigned int PositiveCut, NegativeCut, TotalCall, TotalDepth, CurrentDepth;
		Grail(Graph& graph, int dim);
		~Grail();
		static int visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, int labelid);
		static void randomlabeling(Graph& tree, int labelid);

		void printLabeling(Graph& tree, int i, ostream& out);
		bool reach(int src, int trg);
		bool bidirectionalReach(int src, int trg);
		bool go_for_reach(int src, int trg);
		bool contains(int src, int trg);
		void index_size(int* ind_size);
};

#endif
