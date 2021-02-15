/**
 * @file main.cpp
 * @authors Alessando Ippolito, Federico Maresca
 * @brief 
 * @version 1
 * @date 2021-02-07
 * 
 * @copyright Copyright (c) 2021
 */

#include "Grail.h"

/**
 * @brief Reachability query functions
 */
void searchReachability(Grail&,  ThreadPool&);
//void reachWrapper(Grail&, const std::vector<query>&, const int, const int);

/**
 * @brief Helper functions
 */
static void parseArgs(int, char **, std::string&, std::string&, int&);
static void usage();


// --------------------------------------------------------------------

/*! \mainpage GRAIL WITH CONCURRENT IMPLEMENTATION
* \section intro_sec Introduction
* We use the original paper code " \eGRAIL: A scalable index for reachability queries in very large graphs" by Hilmi Yildirim, Vineet Chaoji
Mohammed J. Zaki found at https://github.com/zakimjz/grail's to implement a version of the program that runs the reading, labeling and reachability
checking in a concurrent manner. 
* \section expl_sec Program explanation
* A threadpool is ran at the start of the execution with a pool size of std::thread::hardware_concurrency(), during the entire execution these
* threads wait on a task queue for their next operation, this queue is protected by condition variables latches which allow only one thread at a time
* to pop a task from the queue. A parse_function reads the input file names and saves it into local variables, all following functions are run by
* pushing the functions into a task queue from which the threads pop it and runs it. 
* The graph read uses the file to read the number of vertexes N and gives each thread N/pool_size vertexes to insert into the graph. 
* The labeling process is run in parallel with each thread executing a traversal
* The query reachabilty operates much like the graph read where each thread receives query_num/pool_size queries to answer.
* Chrono library is used to check on execution time.
*/
int main(int argc, char **argv) {
	std::string filename, testfilename;
	int DIM;
	ThreadPool pool;

	parseArgs(argc, argv, filename, testfilename, DIM);

	/***********************************************
	 * @brief GRAPH FILE INPUT READING
	 */

	cout << "Reading graph..." << endl;

	auto start_read = std::chrono::high_resolution_clock::now();
	Graph graph(filename, pool);
	auto end_read = std::chrono::high_resolution_clock::now();

	/***********************************************
	 * @brief GRAIL CONSTRUCTION AND QUERY READ
	 */

	cout << "Label construction..." << endl;

	auto start_label = std::chrono::high_resolution_clock::now();
	Grail grail(std::ref(graph), DIM,testfilename, pool);
	auto end_label = std::chrono::high_resolution_clock::now();

	/***********************************************
	 * @brief QUERY CONSTRUCTION
	 */

	cout << "Query testing..." << endl;

	auto start_query = std::chrono::high_resolution_clock::now();
	searchReachability(grail, pool);
	auto end_query = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> read_time = end_read - start_read;
	std::chrono::duration<double, std::milli> label_time = end_label - start_label;
	std::chrono::duration<double, std::milli> query_time = end_query - start_query;

	auto program_time = read_time + label_time + query_time;

#if GROUND_TRUTH
	grail.groundTruthCheck(cout);
#endif

	unsigned qsize = grail.getQuerySize();
	unsigned gsize = graph.numVertices();
	unsigned edges = graph.numEdges();

	/***********************************************
	 * @brief String arithmetic to get file names from path
	 */
  	std::string graphname = filename.substr(filename.find_last_of("/\\")+1);
	std::string testname  = testfilename.substr(filename.find_last_of("/\\")+1);
	cout.setf(std::ios::fixed);
	cout.precision(2);
	cout << "\n__________________________________________________\n\n" << endl;
	cout << "GRAIL FILE DATA:" << endl;
	cout << "||Graph file:\t" << graphname << "\n||Test file:\t" <<
			testname << "\n||Traversals:\t" << DIM << endl;
	cout << "Graph has " << gsize << " vertexes and " << edges << " edges.\n" <<
			"Test done on " << qsize << " queries." << endl;
	cout << "\n           #--------------------------#           \n" << endl;
	cout << "GRAIL TIME REPORT:" << endl;
	cout << "-Files Read\t" << read_time.count() << " ms\n-Labeling time\t" <<
			label_time.count()  << " ms\n-Query time\t" << query_time.count() <<
			" ms\n-TOTAL TIME\t"  << program_time.count() << " ms" << endl;
	cout << "\n__________________________________________________\n\n" << endl;

#if DEBUG
	
	std::string path = "./";
	if(std::system("mkdir -p report") == 0)
		path.append("report/");
	else
		cout << "Could not create report folder...";
	if( graphname.find(".") != std::string::npos)
		graphname = graphname.substr(0, graphname.find("."));
	
	ofstream gout(path+graphname+".graph");
	ofstream qout(path+graphname+".query");
	ofstream lout(path+graphname+".label");
	ofstream rout(path+graphname+".reach");


	pool.addJob( [&] { graph.writeGraph(gout); } );
	pool.addJob( [&] { grail.printQuery(qout); } );
	pool.addJob( [&] { grail.printLabeling(lout); });
	pool.addJob( [&] { grail.printReach(rout); });
	pool.waitFinished();
#endif

	return 0;
}

/**
 * @brief Static helper function for command line menu
 * 
 */
static void usage() {		// here we must specify which search we want to implement - probably bidirectional
	cout << "Usage:\n"
			"./grail [-h] <filename> [<DIM>]  <testfilename>\n"
			"\nDescription:\n"
			"- h\t\t\tPrint this help message.\n"
			"- filename\t\tContains the graph to analyze.\n"
			"- DIM (optional)\tNumber of traversal to execute (better no more than 5). If not specified, "
			"equals 2.\n"
			"- testfilename\t\tContains the queries to execute." << endl;
}

/**
 * @brief Standard parser for command line arguments
 * 
 * @param argc 
 * @param argv 
 * @param fname GRAPH_FILENAME This file should contain a graph with the following structure:
 *  n ->number of vertexes
 *  src0 : trg1 trg2 trg3 ... #
 *  src1 : trg1 trg2 trg3 ... # 
 *    .
 *    .
 *    .
 *  srcn: ...
 * @param tfname TEST_FILENAME This file should contain a series of queries with the following structure:
 *  src trg \n src1 trg1 \n ...
 * @param dim #TRAVERSALS This value is the amount of labels or traversals that the Grail should construct 
 */
static void parseArgs(int argc, char **argv, std::string &fname, std::string &tfname, int &dim) {
	if (argc < 3) {	// minimum is ./grail <filename> <testfilename>
		usage();
		exit(EXIT_FAILURE);
	}
 	fname = argv[1];
 	if(isdigit(*argv[2])) {		// ./grail sample.gra 5 sample.que opt(ltype) 
 		dim = atoi(argv[2]);
 		tfname = argv[3];
 	} else {					// ./grail sample.gra sample.que
		dim = 2;
 		tfname = argv[2];
 	}
}


/**
 * @brief This wrapper function handles iteration over all of the queries. It takes
 * a chunk of queries and hands each chunk to the task pool. Each thread then executes this task
 * and accesses the global vector 'reachability' concurrently. 
 * @param grail Grail object
 * @param queries Query vector with all queries from TEST_FILENAME
 * @param pool Threadpool reference used to launch each query search
 */
void searchReachability(Grail &grail, ThreadPool &pool) {
	int size = grail.getQuerySize();
	grail.setReachability(size);
	int begin = 0, chunk = size/CHUNK_N;
	int end;
	while(begin < size) {
		pool.addJob( [=, &grail] { grail.reachWrapper(begin,begin+chunk);} );
		begin += chunk;
		chunk = (begin+chunk < size) ? chunk : size-begin;
	}
	pool.waitFinished();
}


