/*
 * @file Grail.h
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @copyright Copyright (c) 2021
 */

#ifndef GRAIL_H_
#define GRAIL_H_

#include "Graph.h"

typedef std::vector<char> ReachVector;
		
class Grail {
	private:
		Graph& g;
		int dim;
		ReachVector reachability;
	public:
		Grail(Graph& graph, int dim, ThreadPool&);
		~Grail();

		static int visit(Graph& , int , int& , std::vector<bool>& , unsigned short int,std::mt19937& );
		static void randomlabeling(Graph& tree, unsigned short int labelid);
		Graph& getGraph() const {return g;};
		char getReachability(int n) {return reachability[n];}
		void setReach(const int &query_id, const char &sol) {reachability[query_id] = sol;}
		void setReachabilty(const int &n) {reachability.resize(n);};
		char bidirectionalReach(int src, int trg, int query_id, std::vector<int>& visited);
		bool contains(const int &, const int&);
};

void print_labeling(std::ostream &out, Graph &g, int dim);

#endif /* GRAIL_H_ */