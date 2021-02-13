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

struct query{
	int src;
	int trg;
#if GROUND_TRUTH
	int labels = 0;
#endif
};

std::vector<char> reachability;

/**
 * @brief Reachability query functions
 */
void search_reachability(Grail&, const std::vector<query>&, ThreadPool&);
void reachWrapper(Grail&, const std::vector<query>&, const int, const int);

/**
 * @brief Input reading functions
 */
void read_test(const std::string&, std::vector<query>&);

/**
 * @brief Helper functions
 */
static void parse_args(int, char **, std::string&, std::string&, int&);
static void usage();
#if DEBUG
void print_graph(std::ostream&, Graph&);
void print_query(std::ostream&, std::vector<query>&);
void print_reach(std::ostream&, Grail&, std::vector<query>&);
#endif

#if GROUND_TRUTH
void ground_truth_check(const std::vector<query>&);
#endif

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
	std::vector<query> queries;
	ThreadPool pool;

	parse_args(argc, argv, filename, testfilename, DIM);

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
	pool.addJob(read_test, std::ref(testfilename), std::ref(queries));
	Grail grail(std::ref(graph), DIM, pool);
	auto end_label = std::chrono::high_resolution_clock::now();

	/***********************************************
	 * @brief QUERY CONSTRUCTION
	 */

	cout << "Query testing..." << endl;

	auto start_query = std::chrono::high_resolution_clock::now();
	search_reachability(grail, queries, pool);
	auto end_query = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> read_time = end_read - start_read;
	std::chrono::duration<double, std::milli> label_time = end_label - start_label;
	std::chrono::duration<double, std::milli> query_time = end_query - start_query;

	auto program_time = read_time + label_time + query_time;

#if GROUND_TRUTH
	ground_truth_check(queries);
#endif

	unsigned qsize = queries.size();
	unsigned gsize = graph.num_vertices();
	unsigned edges = graph.num_edges();

	/***********************************************
	 * @brief String arithmetic to get file names from path
	 */
  	std::size_t gn = filename.find_last_of("/\\");
	std::size_t tn = testfilename.find_last_of("/\\");
	cout.setf(std::ios::fixed);
	cout.precision(2);
	cout << "\n__________________________________________________\n\n" << endl;
	cout << "GRAIL FILE DATA:" << endl;
	cout << "||Graph file:\t" << filename.substr(gn+1) << "\n||Test file:\t" <<
			testfilename.substr(tn+1) << "\n||Traversals:\t" << DIM << endl;
	cout << "Graph has " << gsize << " vertexes and " << edges << " edges.\n" <<
			"Test done on " << qsize << " queries." << endl;
	cout << "\n           #--------------------------#           \n" << endl;
	cout << "GRAIL TIME REPORT:" << endl;
	cout << "-Files Read\t" << read_time.count() << " ms\n-Labeling time\t" <<
			label_time.count()  << " ms\n-Query time\t" << query_time.count() <<
			" ms\n-TOTAL TIME\t"  << program_time.count() << " ms" << endl;
	cout << "\n__________________________________________________\n\n" << endl;

#if DEBUG
	std::string printpath = "./", queryfile, labelfile, reachfile, graphfile;
	if(std::system("mkdir -p report") == 0)
		printpath.append("report/");
	else
		cout << "Could not create report folder...";
	printpath.append(filename.substr(0, filename.find(".")));
	queryfile = labelfile = reachfile = graphfile = printpath;

	ofstream gout(graphfile.append(".graph"));
	ofstream qout(queryfile.append(".query"));
	ofstream lout(labelfile.append(".label"));
	ofstream rout(reachfile.append(".reach"));

	pool.addJob(print_graph, std::ref(gout), std::ref(graph));
	pool.addJob(print_query, std::ref(qout), std::ref(queries));
	pool.addJob(print_labeling, std::ref(lout), std::ref(graph), DIM);
	pool.addJob(print_reach, std::ref(rout), std::ref(grail), std::ref(queries));
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
static void parse_args(int argc, char **argv, std::string &fname, std::string &tfname, int &dim) {
	if (argc < 3) {	// minimum is ./grail <filename> <testfilename>
		usage();
		exit(EXIT_FAILURE);
	}
 	fname = argv[1];
 	if(isdigit(*argv[2])) {		// ./grail sample.gra 5 sample.que
 		dim = atoi(argv[2]);
 		tfname = argv[3];
 	} else {					// ./grail sample.gra sample.que
		dim = 2;
 		tfname = argv[2];
 	}
}

/**
 * @brief This function reads TEST_FILENAME and saves the queries in a std::vector of queries, a struct
 * that contains 2 integers (src,trg)
 * File structure required:
 *  src  trg opt(label)
 * 	int  int 
 * 	int  int 
 *  ...  ...
 * @param tfname TEST_FILENAME
 * @param queries Query vector
 */
void read_test(const std::string &tfname, std::vector<query> &queries) {
	int src,trg;
	ifstream fstr(tfname);
	if (!fstr) {
		cout << "Error: Cannot open " << tfname << endl;
		exit(EXIT_FAILURE);
	}
#if GROUND_TRUTH
	int label;
		while(fstr >> src >> trg >> label)
			queries.push_back({src, trg, label});
#else
		while(fstr >> src >> trg)
			queries.push_back({src, trg});
#endif
}

#if DEBUG
void print_graph(std::ostream &out, Graph &gr) {
	gr.writeGraph(out);
}

void print_query(std::ostream &out, std::vector<query> &que) {
	for(query &q: que)
#if GROUND_TRUTH
		out << q.src << " " << q.trg << " " << q.labels << endl;
#else
		out << q.src << " " << q.trg << endl;
#endif
}

/**
 * @brief Debugging function, prints the queries 
 * 
 * @param out ostream object as ouput
 * @param grail Grail object that contains the reachability result
 * @param queries queries std::vector of query struct 
 */
void print_reach(std::ostream &out, Grail &grail, std::vector<query> &queries) {
	for (int i=0; i<queries.size(); i++)
		out << queries[i].src << " " << queries[i].trg << " " << reachability[i] << endl;
}
#endif

/**
 * @brief READS THE GRAIL AND STORES IT INTO THE GRAIL OBJECT 
 * This function allows the threadpool to compute a non-static member function by passing its 
 * Object class as a parameter. Each thread runs end-start queries and copies them to their respective
 * vector position concurrently with other vectors, access is thread safe since each vector chunk does not 
 * overlap. Reachability is either Bidirectional or Basic Reach 
 * @param grail Grail object
 * @param src 
 * @param trg 
 * @param query_id Query number to access the std::vector of reachability results
 */
void reachWrapper(Grail &grail, const std::vector<query> &queries, const int start, const int end) {
	std::vector<int> visited(grail.getGraph().num_vertices());
	for (int i=start; i<end; i++)
#if BIDI
		reachability[i] = grail.bidirectionalReach(queries[i].src, queries[i].trg, i, visited);
#else
		reachability[i] = grail.reach(queries[i].src, queries[i].trg, i, visited);
#endif
}

/**
 * @brief This wrapper function handles iteration over all of the queries. It takes
 * a chunk of queries and hands each chunk to the task pool. Each thread then executes this task
 * and accesses the global vector 'reachability' concurrently. 
 * @param grail Grail object
 * @param queries Query vector with all queries from TEST_FILENAME
 * @param pool Threadpool reference used to launch each query search
 */
void search_reachability(Grail &grail, const std::vector<query> &queries, ThreadPool &pool) {
	reachability.resize(queries.size());
	int begin = 0, chunk = queries.size()/CHUNK_N;
	while(begin < queries.size()) {
		pool.addJob(reachWrapper, std::ref(grail), std::ref(queries), begin, begin+chunk);
		begin += chunk;
		chunk = (begin+chunk < queries.size()) ? chunk : queries.size()-begin;
	}
	pool.waitFinished();
}

#if GROUND_TRUTH
/**
 * @brief If a ground truth is present then we need to check the actual result of
 * our reachability report and we use this function to find successes and fails 
 * and then print out a percentage of successes.
 * 
 * @param queries 
 */
void ground_truth_check(const std::vector<query> &queries){
	int i = 0, success = 0, fail = 0;
	for(int i =0; i<queries.size(); i++)
		if( (!queries[i].labels && (reachability[i] == 'f' || reachability[i] == 'n' ))  || ( queries[i].labels && reachability[i] == 'r') )
			success++;
		else fail++;
	cout.precision(2);
	cout << "Success rate: " << success/(success+fail)*100 << "%" << endl;
}
#endif
