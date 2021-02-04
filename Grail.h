/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#ifndef _BOX_H
#define _BOX_H

#include "Graph.h"
#include <stdint.h>
class Grail {
	private:
		Graph& g;
		int dim;
		int QueryCnt;
	public:
		// unsigned int PositiveCut, NegativeCut, TotalCall, TotalDepth, CurrentDepth;
		Grail(Graph& graph, int dim);
		~Grail();
		static int visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited, unsigned short int labelid,
										 std::vector<int>& pre, std::vector<int>& post);
		static void randomlabeling(Graph& tree, unsigned short int labelid);


		void printLabeling(Graph& tree, int i, ostream& out);
		bool bidirectionalReach(int src, int trg);
		bool contains(int src, int trg);
		void index_size(int* ind_size);
};

#endif
