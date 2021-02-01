/* Copyright (c) Hilmi Yildirim 2010,2011.

The software is provided on an as is basis for research purposes.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.
*/
#include "Graph.h"
#include "GraphUtil.h"
#include "Grail.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <csignal>
#include <cstring>
#include <future>
#include <thread>
#include "utils.h"

#define THREADS true

bool BIDIRECTIONAL = false;
int LABELINGTYPE = 0;
int DIM = 2;
char* filename = NULL;
char* testfilename = NULL;
bool debug = false;
bool isquer = false;		// this sould be the way to identify if the file comes from quer or not, right?
float labeling_time, query_time, query_timepart;
int alg_type = 1;

struct query{
	int src;
	int trg;
	int labels;
};

void handle(int sig) {
 	char const *alg_name;

	switch(alg_type){
		case 1: alg_name= "GRAIL";  break;
		case 2: alg_name= "GRAILBI";  break;
	}

	cout << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t" << "TOut" << "\t" <<  endl;

	exit(1);
}

static void usage() {		// here we must specify which search we want to implement - probably bidirectional
	cout << "\nUsage:\n"
		"./grail [-h] <filename> [<DIM>]  <testfilename> \n"
		"Description:\n"
		"<filename> <dim>(int) < testfilename>\n"
		"This program receives a <filename> containing a DAG and creates <DIM> labels\n"
		"which are then queried using the queries found on <testfilename>.\n"
		"If DIM is not specified, its default value is 2.\n"
		<< endl;
}


static void parse_args(int argc, char *argv[]){
	if (argc < 4) {
		usage();
		exit(0);
	}
 	int i = 1;
 	filename = argv[1];
 	DIM = atoi(argv[2]);
 	testfilename = argv[3];
}

/*
 * ADDED FUNCTIONS
 *
 * This reads the queries file
 */

void read_test(vector<query> &q) {
	cout << "preparing queries..." << endl;
	int s,t,label=0;
	ifstream fstr(testfilename);
	if(isquer)
		while(fstr >> s >> t)
			q.push_back({s,t,label});
	else
		while(fstr >> s >> t >> label)
			q.push_back({s,t,label});
	cout << "queries are ready" << endl;
}

void print_test(vector<query> queries) {
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
	for (auto &q : queries) {
		outfile <<  q.src << " " << q.trg << endl;
	}
}

#if THREADS

/*
 * THIS FUNCTION RETURNES AS A FUTURE THE READ GRAPH
 */
void read_graph(promise<Graph>& pgraph){
	ifstream infile(filename);
	if (!infile) {
		cout << "Error: Cannot open " << filename << endl;
		return ;
	}
	//Run a thread to read
	Graph g(infile);

	/*
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
	g.writeGraph(outfile);*/
	pgraph.set_value(g);
}

#endif

/*
 * **********************************
 */

int main(int argc, char* argv[]) {
	signal(SIGALRM, handle);
	parse_args(argc,argv);

	/*
	 *	Read Graph from the input file AND prepare queries
	 */
	srand48(time(NULL));
	struct timeval after_time, before_time, after_timepart;
	gettimeofday(&before_time, NULL);

	vector<query> queries;

#if THREADS

	/*
	 * Graph reading thread returns the graph as a promise
	 */
	promise<Graph> pgraph;
	future<Graph> fgraph = pgraph.get_future();
	std::thread readGraphThread(read_graph,ref(pgraph));
	std::thread testfileThread(&read_test, ref(queries));

	testfileThread.join();
	/*
	 * While the graph is running run a thread that reads the test file
	 */


	Graph g = fgraph.get();
	readGraphThread.join();

#else

	ifstream infile(filename);
	if (!infile) {
		cout << "Error: Cannot open " << filename << endl;
		return 1;
	}

	Graph g(infile);

	read_test(queries);

#endif

	/*
	 * Time check evaluation
	 */

	gettimeofday(&after_time, NULL);
	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#graph read time: " << labeling_time << " (ms)" << endl;

	// ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.gra");
	// g.writeGraph(outfile);
	//print_test(queries);
	cout << "#vertex size: " << g.num_vertices() << "\t#edges size: " << g.num_edges() << endl;

	int gsize = g.num_vertices();
	bool r;




	/*
	 * Labeling happens here
	 */
	gettimeofday(&before_time, NULL);

	Grail grail(g, DIM);

	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#construction time: " << labeling_time << " (ms)" << endl;



	/*
	 * Query processing happens here
	 */
	cout << "process queries..." << endl;
	gettimeofday(&before_time, NULL);

	int source, target;
	int reachable = 0, nonreachable =0;
	// vector<query>::iterator q;
	int success = 0, fail = 0;

	/*
	 * Let's put the false positives and negatives in respective files
	 * for this I created a 'Reports' folder
	 */
	ofstream falsepos("../Reports/falsepositives.que");
	ofstream falseneg("../Reports/falsenegatives.que");
	for (auto &q : queries) {		// let's just make sure Savino likes this...
		switch(alg_type) {
			case 1: r = grail.reach(q.src, q.trg); break;
			case 2: r = grail.bidirectionalReach(q.src, q.trg); break;
		}
		if(isquer){
			(r==true) ? reachable++ : nonreachable++;
		} else {
			if(r==true) {
				reachable++;
				success++;
				if(q.labels == 0) {
					falsepos << "False positive pair = " << q.src << " " << q.trg << " " << q.labels << endl;
					falsepos << "Levels : " << q.src  << "->" << g[q.src ].top_level << " " << q.trg << "->" << g[q.trg].top_level << endl;
					fail++;
					success--;
				}
			} else {
				nonreachable++;
				success++;
				if(q.labels == 1) {
					falseneg << "False negative pair = " << q.src  << " " << q.trg << " " << q.labels << endl;
					fail++;
					success--;
				}
			}
		}
	}

	falsepos.close();
	falseneg.close();

	cout << "all done\n\nSuccess Rate " << success << "/" << success+fail <<
			"\nCheck 'falsepositive.que' and 'falsenegative.que' files for more details.\n"
			<< endl;

	gettimeofday(&after_time, NULL);
	query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#total query running time: " << query_time << " (ms)" << endl;

	cout.setf(ios::fixed);		
	cout.precision(2);
	cout << "GRAIL REPORT " << endl;
	cout << "filename = " << filename << "\t testfilename = " << testfilename <<
			"\t DIM = " << DIM << endl;
	cout << "Labeling_time = " << labeling_time  << "\t Query_Time = " << query_time <<
			"\t Index_Size = " << gsize*DIM*2  << endl;
	
	char const *alg_name;

	switch(alg_type) {
		case 1: alg_name= "GRAIL";  break;
		case 2: alg_name= "GRAILBI";  break;
	}
	
	/* I'm commenting this since there is no comparison to be done
	 * we're just using one algorithm and one labeling type
	cout << "COMPAR:\n" <<
			"Algorithm name: " << alg_name << "\n"
			"Traversals number: " << DIM << "\n"
			"Labeling time: " << labeling_time << "\n"
			"Query time: " << query_time << "\n"
			"Total call: " << grail.TotalCall << "\n"	// What is this for?!
			"Reachable: " << reachable << endl;
	*/
}
