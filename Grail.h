/*
 * @file Grail.h
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @c
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
 * Contains a pointer to graph 
 * number of labelings
 * reachability results 
 * query questions
 */
class Grail {
	private:
		Graph& g;
		int dim;
		std::vector<char> reachability;
		std::vector<query> queries;
	public:
		
		

		Grail(Graph&, const int, const std::string, ThreadPool&);
		~Grail();

		void randomlabeling(const unsigned short);
		int visit(const int, int&, std::vector<bool>&, const unsigned short, std::mt19937&);
		bool contains(const int, const int);

		char bidirectionalReach(const int, const int, int, std::vector<int>&);
		void reachWrapper(const int start, const int end);
		void readQueries(const std::string& );

		int  getQuerySize() const ;
		void setReachability(const int);
#if DEBUG
		void printLabeling(std::ostream& );
		void printReach(std::ostream& );
		void printQuery(std::ostream& );
#endif

#if GROUND_TRUTH
		void groundTruthCheck(std::ostream& );
#endif
};


#endif /* GRAIL_H_ */
