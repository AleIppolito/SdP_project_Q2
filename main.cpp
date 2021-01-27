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

#include "utils.h"


bool SKIPSCC = true; // graph will always be a DAG
bool BIDIRECTIONAL = false;
int LABELINGTYPE = 0;
int DIM = 2;
int query_num = 100000;
char* filename = NULL;
char* testfilename = NULL;
bool debug = false;

float labeling_time, query_time, query_timepart;
int alg_type = 1;

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
		" grail [-h] <filename> -test <testfilename> [-dim <DIM>] [-ltype <labelingtype>] [-t <alg_type>]\n"
		"Description:\n"
		"	-h			Print the help message.\n"
		"  <filename> is the name of the input graph in gra format.\n"
		"	-test		Set the queryfile which contains a line of <source> <target> <reachability> for each query. \n"
		"	-dim 		Set the number of traversals to be performed. The default value is 5.\n"
		"	-t <alg_type>		alg_type can take 2 different values.  Default value is 1.\n"
		" \t\t\t 1  : Basic Search (used in the original VLDB'10 paper) \n"
		" \t\t\t 2  : Bidirectional Search \n"
		"	-ltype <labeling_type>		labeling_type can take 2 different values.  Default value is 0.\n"
		" \t\t\t 0  : Completely randomized traversals.  \n"
		" \t\t\t 1  : Randomized Reverse Pairs Traversals (usually provides the best quality) \n"
		<< endl;
}


static void parse_args(int argc, char *argv[]){
	if (argc == 1) {
		usage();
		exit(0);
	}

 	int i = 1;

	while (i < argc) {
		if (strcmp("-h", argv[i]) == 0) {
			usage();
			exit(0);
		}
		else if (strcmp("-dim", argv[i]) == 0) {
			i++;
			DIM = atoi(argv[i++]);
		}else if (strcmp("-test", argv[i]) == 0) {
			i++;
			testfilename = argv[i++];
		}else if(strcmp("-ltype", argv[i])== 0) {
			i++;
			LABELINGTYPE = atoi(argv[i++]);
		}else if(strcmp("-t", argv[i])== 0) {
			i++;
			alg_type = atoi(argv[i++]);
		}else if(strcmp("-bi", argv[i])== 0) {
			i++;
			BIDIRECTIONAL = true;
			alg_type *=3;
		}else {
			filename = argv[i++];
		}
	}
	if(!testfilename){
		cout << "Please provide a test file : -test <testfilename> " << endl;
		exit(0);
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
	
	Graph g(infile);
	cout << "#vertex size:" << g.num_vertices() << "\t#edges size:" << g.num_edges() << endl;

	int s, t;
	int left = 0;
	int gsize = g.num_vertices();
	
	bool r;
	struct timeval after_time, before_time, after_timepart, before_timepart;



	// prepare queries
	srand48(time(NULL));
	cout << "preparing queries..." << endl;
	vector<int> src;
	vector<int> trg;
	vector<char> labels;
	vector<int>::iterator sit, tit;
	vector<char>::iterator lit;
	int success=0,fail=0;
	int label;
      std::ifstream fstr(testfilename);
		while(fstr >> s >> t >> label) {
			src.push_back(s);
			trg.push_back(t);
			labels.push_back(label);
		}

	cout << "queries are ready" << endl;


	gettimeofday(&before_time, NULL);

	/**************
	 * Labeling happens here
	 */
	Grail grail(g,DIM,LABELINGTYPE);

	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#construction time:" << labeling_time << " (ms)" << endl;
	


	/**************
	 * Query processing happens here
	 */
	cout << "process queries..." << endl;
	gettimeofday(&before_time, NULL);
	gettimeofday(&before_timepart, NULL);
	int seenpositive = 0;

	int source, target;
	int reachable = 0, nonreachable =0;
		
	for (sit = src.begin(), tit = trg.begin(), lit = labels.begin();sit != src.end(); ++sit, ++tit, ++lit) {

			s = *sit;
			t = *tit;


		switch(alg_type){
			case 1: r = grail.reach(s,t); break;
			case 2: r = grail.bidirectionalReach(s,t); break;
		}

		if(r==true) {
			reachable++;
			if(*lit == '0') {
				cout << "False positive pair = " << s << " " << t << " " << *lit << endl;
				cout << "Levels : " << s << "->" << g[s].top_level << " " << t << "->" << g[t].top_level << endl;
				fail++;
			} else {
				success++;
			}
		} else {
			nonreachable++;
			if(*lit == '1') {
				cout << "False negative pair = " << s << " " << t << " " << *lit << endl;
				fail++;
			} else {
				success++;
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

	if(grail.PositiveCut==0)
		grail.PositiveCut = 1;	
	
	int totalIndexSize=gsize*DIM*2;
	
	
	cout << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t" << query_time << "\t" <<  totalIndexSize << "\t" << print_mem_usage()  << "\t" << grail.TotalCall << "\t" << grail.PositiveCut << "\t" << grail.NegativeCut << "\t" << reachable << "\t AvgCut:" << (grail.TotalDepth/grail.PositiveCut) << endl;
}
