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

#include <thread>
#include "utils.h"


bool BIDIRECTIONAL = false;
int LABELINGTYPE = 0;
int DIM = 2;
char* filename = NULL;
char* testfilename = NULL;
bool debug = false;
bool isquer =true;
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

static void usage() {
	cout << "\nUsage:\n"
		" grail [-h] <filename> [<DIM>  <testfilename> \n"
		"Description:\n"
		"<filename> <dim>(int) < testfilename>"
		"This program receives a file containin a DAG and creates dim labels\n"
		"which are then queried using the queries found on <testfilename>\n"
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


void read_test(	std::vector<query>& q  ) {

	int s,t,label=0;
	std::ifstream fstr(testfilename);
	if(isquer){
		while(fstr >> s >> t) {
			q.push_back({s,t,label});
			}
	}else {
		while(fstr >> s >> t >> label) {
			q.push_back({s,t,label});
			}
		}

}
void print_test(std::vector<query> queries){
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
	for (auto &q : queries) {
		outfile <<  q.src << " " << q.trg << endl;
	}
}


int main(int argc, char* argv[]) {
	signal(SIGALRM, handle);	
	parse_args(argc,argv);
		
	/*
		Read Graph from the input file	
	*/
	ifstream infile(filename);
	if (!infile) {
		cout << "Error: Cannot open " << filename << endl;
		return -1;
	}

	srand48(time(NULL));
	struct timeval after_time, before_time, after_timepart;
	gettimeofday(&before_time, NULL);
	//Run a thread to read
	Graph g(infile);
	cout << "#vertex size:" << g.num_vertices() << "\t#edges size:" << g.num_edges() << endl;
	gettimeofday(&after_time, NULL);

		labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
		cout << "#graph read time:" << labeling_time << " (ms)" << endl;
	/*
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
	g.writeGraph(outfile);*/

	int gsize = g.num_vertices();
	
	bool r;



	// prepare queries
	cout << "preparing queries..." << endl;
	std::vector<query> queries;

	/*
	 * Thread reads testfile
	 * TODO parallel read of infile and testfile
	 */
	std::thread testfileThread(&read_test, std::ref(queries));
	testfileThread.join();
	//print_test(queries);
	cout << "queries are ready" << endl;

	/**************
	 * Labeling happens here
	 */
	gettimeofday(&before_time, NULL);

	Grail grail(g,DIM);

	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#construction time:" << labeling_time << " (ms)" << endl;



	/**************
	 * Query processing happens here
	 */
	cout << "process queries..." << endl;
	gettimeofday(&before_time, NULL);

	int source, target;
	int reachable = 0, nonreachable =0;
	std::vector<query>::iterator q;
	int success=0,fail=0;

	for (auto &q : queries) {

		switch(alg_type){
			case 1: r = grail.reach(q.src,q.trg); break;
			case 2: r = grail.bidirectionalReach(q.src,q.trg); break;
		}
		if(isquer){
			(r==true) ? reachable++ : nonreachable++;
		}
		else{
			if(r==true) {
				reachable++;
				if(q.labels == 0) {
					cout << "False positive pair = " << q.src << " " << q.trg << " " << q.labels << endl;
					cout << "Levels : " << q.src  << "->" << g[q.src ].top_level << " " << q.trg << "->" << g[q.trg].top_level << endl;
					fail++;
				} else {
					success++;
				}
			} else {
				nonreachable++;
				if(q.labels == 1) {
					cout << "False negative pair = " << q.src  << " " << q.trg << " " << q.labels << endl;
					fail++;
				} else {
					success++;
				}
			}
		}
	}

	cout << "Success Rate " << success << "/" << success+fail << endl;

	gettimeofday(&after_time, NULL);
	query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#total query running time:" << query_time << " (ms)" << endl;

	cout.setf(ios::fixed);		
	cout.precision(2);
	cout << "GRAIL REPORT " << endl;
	cout << "filename = " << filename << "\t testfilename = " << (testfilename ? testfilename:"Random") << "\t DIM = " << DIM <<endl;  
	cout << "Labeling_time = " << labeling_time  << "\t Query_Time = " << query_time << "\t Index_Size = " << gsize*DIM*2  << "\t Mem_Usage = " << print_mem_usage() << " MB"<< endl;
	
	char const *alg_name;

	switch(alg_type){
		case 1: alg_name= "GRAIL";  break;
		case 2: alg_name= "GRAILBI";  break;
	}

	
	
	cout << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t" << query_time << "\t" << "\t" << print_mem_usage() << "\t" << grail.TotalCall << "\t" << reachable << endl;
}
