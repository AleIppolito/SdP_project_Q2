
/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Grail.h"
<<<<<<< Updated upstream
// #include <csignal>
// #include <stdlib.h>
// #include <stdio.h>
// #include <signal.h>
// #include <cstring>
// #include <thread>

// bool BIDIRECTIONAL = true;
// int LABELINGTYPE = 0;
// bool debug = false;
=======
>>>>>>> Stashed changes
int DIM = 2;
char* filename = NULL;
char* testfilename = NULL;
bool isquer = false;
<<<<<<< Updated upstream
float labeling_time, query_time, query_timepart;
int alg_type = 2;

=======
float labeling_time, query_time;
#include <unistd.h>
>>>>>>> Stashed changes
struct query{
	int src;
	int trg;
	int labels;
};

<<<<<<< Updated upstream
/*
void handle(int sig) {
 	char const *alg_name;

	switch(alg_type){
		case 1: alg_name= "GRAIL";  break;
		case 2: alg_name= "GRAILBI";  break;
	}

	cout << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t" << "TOut" << "\t" <<  endl;

	exit(1);
}
*/
=======
>>>>>>> Stashed changes

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

<<<<<<< Updated upstream
/*
 * ADDED FUNCTIONS
 *
 **************************************
 *
 * This reads the queries file
 */

=======
#if DEBUG

void print_test(std::vector<query> queries) {
	cout << "printing test " << endl;
	ofstream outfile("./test");
	if (isquer){
		for (auto &q : queries) {
			outfile <<  q.src << " " << q.trg << endl;}}
	else{
		for (auto &q : queries) {
			outfile <<  q.src << " " << q.trg << " " << q.labels << endl;}}
	}

#endif
// Read testfilename and prepare queries vector
>>>>>>> Stashed changes
void read_test(std::vector<query> &q) {
	int s,t,label=0;
	ifstream fstr(testfilename);
	if(!fstr){
		cout << "Error: Cannot open " << testfilename << endl;
		return;
	}
	if(isquer){
		while(fstr >> s >> t >> std::ws){
			q.push_back({s,t,label});
		}
	}else{
		while(fstr >> s >> t >> label >> std::ws){
			q.push_back({s,t,label});
		}	
	}
		
	#if DEBUG
	print_test(q);
	#endif
}
#if THREADS
void read_graph(Graph& g, ThreadPool &p){
	g.readGraph(filename,p);
}
#endif

<<<<<<< Updated upstream
#if DEBUG
void print_test(std::vector<query> queries) {
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
=======
void Wbidirectional(Grail &grail, int src, int trg, int query_id){
	grail.bidirectionalReach(src,trg,query_id);
}

void print_query(ostream & out,Grail& grail, std::vector<query> queries){
	int i = 0;
>>>>>>> Stashed changes
	for (auto &q : queries) {
		out << q.src << " " << q.trg << " " << grail.getReachability(i++) << endl;
	}
}
<<<<<<< Updated upstream
#endif

#if THREADS

/*
 * THIS FUNCTION RETURNES AS A FUTURE THE READ GRAPH
 */
void read_graph(std::promise<Graph>& pgraph){
	Graph g(filename);
	pgraph.set_value(g);
}
#endif
=======
void search_reachability(Grail& grail, std::vector<query> queries, ThreadPool &pool){
	grail.setReachabilty(queries.size());
	int i =0;
	for (auto &q : queries) {
		pool.addJob(Wbidirectional, std::ref(grail),q.src, q.trg, i);
		i++;
		usleep(500);
	}
	pool.waitFinished();
}
// --------------------------------------------------------------------
>>>>>>> Stashed changes

/*
 * **********************************
 */

int main(int argc, char* argv[]) {
	// signal(SIGALRM, handle);
	parse_args(argc,argv);
	/*
	 *	Read Graph from the input file AND prepare queries
	 */

	srand48(time(NULL));
	struct timeval after_time, before_time, after_timepart;
	gettimeofday(&before_time, NULL);

	std::vector<query> queries;

#if THREADS
<<<<<<< Updated upstream
	// Graph reading thread returns the graph as a promise
	std::promise<Graph> pgraph;
	std::future<Graph> fgraph = pgraph.get_future();
	std::thread readGraphThread(read_graph,std::ref(pgraph));
	std::thread testfileThread(&read_test, std::ref(queries));


	// While the graph is running run a thread that reads the test file
	Graph g = fgraph.get();
	readGraphThread.join();
	testfileThread.join();
=======

	Graph g;
	pool.addJob(read_graph, std::ref(g), std::ref(pool));
	pool.addJob(read_test, std::ref(queries));
	pool.waitFinished();		// synch point
	
	//Graph g(filename);
>>>>>>> Stashed changes
#else

	Graph g(filename);

	read_test(queries);

#endif
<<<<<<< Updated upstream

	/*
	 * Time check evaluation
	 */

=======
	
	// Time check evaluation
>>>>>>> Stashed changes
	gettimeofday(&after_time, NULL);
	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#graph read time: " << labeling_time << " (ms)" << endl;
	cout << "#vertex size: " << g.num_vertices() << "\t#edges size: " << g.num_edges() << endl;
	ofstream out("./out");
	g.writeGraph(out);
<<<<<<< Updated upstream
	/*
	 * Labeling happens here
	 */
	cout << "Starting GRAIL labeling... " << endl;
	gettimeofday(&before_time, NULL);

	Grail grail(g, DIM);

=======

#endif
	// Labeling happens here
	cout << "starting GRAIL labeling..." << endl;
	gettimeofday(&before_time, NULL);
#if THREADS
	Grail grail(std::ref(g), DIM, pool);
	
#else
	Grail grail(g, DIM);
#endif
>>>>>>> Stashed changes
	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#Grail construction time: " << labeling_time << " (ms)" << endl;

	//print_graph(std::ref(g),"./label", DIM);
	#if DEBUG
	
	#endif
	/*
	 * Query processing happens here
	 */

	cout << "Starting reachability testing..." << endl;
	gettimeofday(&before_time, NULL);
	/*
	 * Let's put the false positives and negatives in respective files,
	 * For this I created a 'Reports' folder
	 */


	/*
	 * Here we use auto, but let's just make sure Savino likes this, otherwise we'll use a vector iterator
	 */
	search_reachability(std::ref(grail), queries, std::ref(pool));

	gettimeofday(&after_time, NULL);
	query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#total query running time: " << query_time << " (ms)" << endl;
	//print_query(cout,grail,queries);
	cout.setf(ios::fixed);		
	cout.precision(2);
	cout << "GRAIL REPORT " << endl;
	cout << "filename = " << filename << "\t testfilename = " << testfilename <<
			"\t DIM = " << DIM << endl;
	cout << "Labeling_time = " << labeling_time  << "\t Query_Time = " << query_time <<
			"\t Index_Size = " << g.num_vertices()*DIM*2  << endl;
}
