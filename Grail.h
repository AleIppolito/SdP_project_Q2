/* Copyright (c) Hilmi Yildirim 2010,2011.

The software is provided on an as is basis for research purposes.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.
*/
#ifndef _BOX_H
#define _BOX_H

#include "GraphUtil.h"
#include "exception_list.h"

// test switch
#define _TEST_

class Grail {
	public:
		Graph& g;
		struct timeval after_time, before_time;
		float run_time;
		int dim;
		int *visited;
		int QueryCnt;
		bool LEVEL_FILTER;
		bool POOL;
		int POOLSIZE;
		unsigned int PositiveCut, NegativeCut, TotalCall, TotalDepth, CurrentDepth;
	public:
		Grail(Graph& graph, int dim);
		Grail(Graph& graph, int dim, int labelingType, bool POOL, int POOLSIZE);
		~Grail();
		static int visit(Graph& tree, int vid, int& pre_post, vector<bool>& visited);
		static int fixedreversevisit(Graph& tree, int vid, int& pre_post, vector<bool>& visited,int traversal);
		static void randomlabeling(Graph& tree);
		static void fixedreverselabeling(Graph& tree, int traversal);
		static void setIndex(Graph& tree, int traversal); 

		void set_level_filter(bool lf);
		bool reach(int src, int trg, ExceptionList * el);
		bool reach_lf(int src, int trg, ExceptionList * el);
		bool go_for_reach(int src, int trg);
		bool go_for_reach_lf(int src, int trg);
		bool contains(int src, int trg);
		void index_size(int* ind_size);
};

#endif
