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

struct query{
	int src;
	int trg;
#if GROUND_TRUTH
	int labels = 0;
#endif
};



/**
 * @brief Class Grail
 * Contains a pointer to graph and number of labelings 
 */
class Grail {
	private:
		Graph& g;
		int dim;
	public:
		std::vector<char> reachability;
		std::vector<query> queries;

		Grail(Graph&, const int, const std::string, ThreadPool&);
		~Grail();

		void randomlabeling(const unsigned short);
		int visit(const int, int&, std::vector<bool>&, const unsigned short, std::mt19937&);
		bool contains(const int, const int);
#if BIDI
		char bidirectionalReach(const int, const int, int, std::vector<int>&);
#else
		char reach(const int, const int, int, std::vector<int>&);
		char go_for_reach(const int, const int, int, std::vector<int>&);
#endif
		void reachWrapper(const int start, const int end);
		void readQueries(const std::string& );

#if DEBUG
		void print_labeling(std::ostream& );
		void print_reach(std::ostream& );
		void print_query(std::ostream& );
#endif

#if GROUND_TRUTH
		void ground_truth_check(std::ostream& );
#endif
		void setReachability(const int);
		Graph& getGraph() const;
};


#endif /* GRAIL_H_ */
