/**
 * @file main.cpp
 * @authors Federico Maresca, Alessando Ippolito
 * @brief 
 * @version 1
 * @date 2021-02-07
 * 
 * @copyright Copyright (c) 2021
 */

#include "Grail.h"

bool isquer = false;
float graph_time, labeling_time, query_time;
struct query{
	int src;
	int trg;
	int labels;
};
std::vector<char> reachability;
void search_reachability(Grail &grail, const std::vector<query> &queries, ThreadPool &pool);
void Wbidirectional(Grail &grail, const std::vector<query> &queries, int start, int end);
void print_query(ostream &out, Grail &grail, std::vector<query> &queries);
void read_graph(const std::string &fname, Graph &gr);
void read_test(const std::string &tfname, std::vector<query> &q);
static void parse_args(int argc, char *argv[], std::string &fname, std::string &tfname, int &dim);
static void usage();

// --------------------------------------------------------------------

/* GRAIL WITH CONCURRENT IMPLEMENTATION
* A threadpool is ran at the start of the execution with a pool size of std::thread::hardware_concurrency()
* we then use a parse_function to read the input and save it into local variables, all following functions are run by
* pushing the functions into a task queue from which the threads pop it and runs it.
* Chrono library is used to check on execution time.
*/
int main(int argc, char* argv[]) {
	auto program_start = std::chrono::high_resolution_clock::now();

	std::string filename, testfilename;
	int DIM = 2;
	std::vector<query> queries;
	ThreadPool pool;

	parse_args(argc, argv, filename, testfilename, DIM);

	/**
	 * @brief FILE INPUT READING happens concurrently for both files
	 */
	cout << "Reading input..." << endl;

	auto start_read = std::chrono::high_resolution_clock::now();

	Graph g;
	pool.addJob(read_graph, std::ref(filename), std::ref(g));
	pool.addJob(read_test, std::ref(testfilename), std::ref(queries));
	pool.waitFinished();

	auto end_read = std::chrono::high_resolution_clock::now();

#if DEBUG
	ofstream qout("./queries");
	cout << "print_queries_and_nodes" << endl;
	for(query &q: queries)
		qout << q.src << " " << q.trg << " " << q.labels << endl;
	qout.close();

	ofstream gout("./graph");
	g.writeGraph(gout);
#endif

	/**
	 * @brief GRAIL CONSTRUCTION, each traversal happens on a different thread
	 */
	cout << "Label construction..." << endl;

	auto start_label = std::chrono::high_resolution_clock::now();

	Grail grail(std::ref(g), DIM, pool);

	auto end_label = std::chrono::high_resolution_clock::now();


	/**
	 * @brief QUERY CONSTRUCTION each query check happens on a differnt thread
	 */

	cout << "Query testing..." << endl;

	auto start_query = std::chrono::high_resolution_clock::now();

	search_reachability(std::ref(grail),std::ref(queries), std::ref(pool));

	auto end_query = std::chrono::high_resolution_clock::now();

	unsigned int size = g.num_vertices();
	unsigned int edges = g.num_edges();
	//g.clear();
	std::chrono::duration<double, std::milli> read_time = end_read - start_read;
	std::chrono::duration<double, std::milli> label_time = end_label - start_label;
	std::chrono::duration<double, std::milli> query_time = end_query - start_query;
	auto program_time = read_time + label_time + query_time;


	/**
	 * @brief Take file names from file path then used at line 233
	 */
  	std::size_t gn = filename.find_last_of("/\\");
	std::size_t tn = testfilename.find_last_of("/\\");
	cout.setf(ios::fixed);
	cout.precision(3);
	cout << "\n__________________________________________________\n\n" << endl;
	cout << "GRAIL FILE DATA:" << endl;
	cout << "||Graph file: " << filename.substr(gn+1) << "||\n||Test file: " <<
			testfilename.substr(tn+1) << "||Traversals : " << DIM << endl;
	cout << "Graph has:\nVertexes: " << size << " Edges: " << edges << " Queries: " <<
			queries.size() << endl;
	cout << "\n#-------------------------#\n" << endl;
	cout << "GRAIL TIME REPORT:" << endl;
	cout << "-Files Read\t" << read_time.count() << " ms\n-Labeling time\t" <<
			label_time.count()  << " ms\n-Query time\t" << query_time.count() <<
			" ms\n-TOTAL TIME\t"  << program_time.count() << " ms" << endl;
	cout << "\n__________________________________________________\n\n" << endl;

	//#if DEBUG
	ofstream out("./queriesour.txt");
	print_query(out, grail, queries);
	//#endif
}

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
 *  src : trg1 trg2 trg3 ... #
 *  src : trg1 trg2 trg3 ... # 
 * @param tfname TEST_FILENAME This file should contain a series of queries with the following structure:
 *  src trg \n src1 trg1 \n ...
 * @param dim #TRAVERSALS This value is the amount of labels or traversals that the Grail should construct 
 */
static void parse_args(int argc, char *argv[], std::string &fname, std::string &tfname, int &dim) {
	if (argc < 3) {	// minimum is ./grail <filename> <testfilename>
		usage();
		exit(0);
	}
 	fname = argv[1];
 	if(isdigit(*argv[2])) {
 		dim = atoi(argv[2]);
 		tfname = argv[3];
 	} else
 		tfname = argv[2];
}

/**
 * @brief This function reads TEST_FILENAME and saves the queries in a std::vector of queries, a struct
 * that contains 2 integers (src,trg)
 * 
 * @param tfname TEST_FILENAME
 * @param q Query vector
 */
void read_test(const std::string &tfname, std::vector<query> &q) {
	int s,t,label=0;
	ifstream fstr(tfname);
	if (!fstr) {
		cout << "Error: Cannot open " << tfname << endl;
		return ;
	}
	if(isquer)
		while(fstr >> s >> t)
			q.push_back({s, t, 0});
	else
		while(fstr >> s >> t >> label)
			q.push_back({s, t, label});
}

/**
 * @brief READS THE GRAPH AND STORES IT INTO THE GRAPH OBJECT
 * This wrapper function allows the threadpool to compute a non-static member function by passing its 
 * Object class as a parameter. 
 * 
 * @param fname GRAPH_FILENAME
 * @param gr Graph Object
 */
void read_graph(const std::string &fname, Graph &gr) {
	gr.readGraph(fname);
}

/**
 * @brief Debugging function, prints the queries 
 * 
 * @param out ostream object as ouput
 * @param grail Grail object that contains the reachability result
 * @param queries queries std::vector of query struct 
 */
void print_query(ostream &out, Grail &grail, std::vector<query> &queries) {
	int i = 0;
	cout << "Queries solution: " << endl;
	for (auto &q : queries){
		out << q.src << " " << q.trg << " " << reachability[i] << endl;
		i++;
	}
}

/**
 * @brief READS THE GRAIL AND STORES IT INTO THE GRAIL OBJECT 
 * This wrapper function allows the threadpool to compute a non-static member function by passing its 
 * Object class as a parameter.
 * 
 * @param grail Grail object
 * @param src 
 * @param trg 
 * @param query_id Query number to access the std::vector of reachability results
 */
void Wbidirectional(Grail &grail, const std::vector<query> &queries, int start, int end) {
	std::vector<char> localreach;
	std::vector<int> visited(grail.getGraph().num_vertices());
	for (int i = start; i < end; i++) {
		reachability[i] = grail.bidirectionalReach(queries[i].src,queries[i].trg,i,std::ref(visited));		
	}	
}

/**
 * @brief This wrapper function handles iteration over all of the queries. Each label is checked through 
 * bidirecitonalReach in Grail.cpp  
 * 
 * @param grail Grail object
 * @param queries Query vector with all queries from TEST_FILENAME
 * @param pool Threadpool reference used to launch each query search
 */
void search_reachability(Grail &grail, const std::vector<query> &queries, ThreadPool &pool) {
	//grail.setReachabilty(queries.size());
	reachability.resize(queries.size());
	int begin = 0;
	int chunk = 90000;
	while(begin < queries.size()) {
		pool.addJob(Wbidirectional, std::ref(grail),std::ref(queries),begin,begin+chunk);
		begin +=chunk;
		chunk = (begin+chunk < queries.size()) ? chunk : queries.size()-begin;
	}
	pool.waitFinished();
}
