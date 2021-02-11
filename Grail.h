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
		
class Grail {
	private:
		Graph& g;
		int dim;
	public:
		Grail(Graph&, const int, ThreadPool&);
		~Grail();

		static void randomlabeling(Graph&, const unsigned short);
		static int visit(Graph&, const int, int&, std::vector<bool>&, const unsigned short, std::mt19937&);
		bool contains(const int, const int);
#if BIDI
		char bidirectionalReach(const int, const int, int, std::vector<int>&);
#else
		char reach(const int, const int, int, std::vector<int>&);
		char go_for_reach(const int, const int, int, std::vector<int>&);
#endif
		char getReachability(const int n);
		Graph& getGraph() const;
};

void print_labeling(std::ostream&, Graph&, const int);

#endif /* GRAIL_H_ */
