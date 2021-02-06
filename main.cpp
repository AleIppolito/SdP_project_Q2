
/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Grail.h"
int DIM = 2;
char* filename = NULL;
char* testfilename = NULL;
bool isquer = false;
float labeling_time, query_time, query_timepart;
#include <unistd.h>
struct query{
	int src;
	int trg;
	int labels;
};

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
	if (argc < 3) {	// minimum is ./grail <filename> <testfilename>
		usage();
		exit(0);
	}
 	filename = argv[1];
 	if(isdigit(*argv[2])) {
 		DIM = atoi(argv[2]);
 		testfilename = argv[3];
 	} else
 		testfilename = argv[2];
}


/*
 * ADDED FUNCTIONS
 *
 **************************************
 *
 * This reads the queries file
 */
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

#if DEBUG

void print_test(std::vector<query> queries) {
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");





#endif
void print_query(ostream & out,Grail& grail, std::vector<query>& queries){
	int i = 0;
	cout << "Queries solution : " << endl;
	for (auto &q : queries) {
		out << q.src << " " << q.trg << " " << grail.getReachability(i++) << endl;
	}
}
void Wbidirectional(Grail &grail, int src, int trg, int query_id){
	grail.bidirectionalReach(src,trg,query_id);
}
void search_reachability(Grail& grail, std::vector<query> queries, ThreadPool &pool){
	grail.setReachabilty(queries.size());
	int i =0;
	for (auto &q : queries) {
		pool.addJob(Wbidirectional, std::ref(grail),q.src, q.trg, i);
		i++;
	}
	pool.waitFinished();
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------

int main(int argc, char* argv[]) {
	parse_args(argc, argv);

#if THREADS
	ThreadPool pool(std::thread::hardware_concurrency());
#endif

	// Read Graph from the input file AND prepare queries
	std::vector<query> queries;
	srand48(time(NULL));	// random init
	struct timeval after_time, before_time, after_timepart;
	gettimeofday(&before_time, NULL);

#if THREADS

	Graph g;
	pool.addJob(read_graph, std::ref(g), std::ref(pool));
	pool.addJob(read_test, std::ref(queries));
	pool.waitFinished();		// synch point
	
	//Graph g(filename);

#else
	read_test(queries);
	Graph g(filename);
#endif

	// Time check evaluation
	gettimeofday(&after_time, NULL);
	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#input files read time: " << labeling_time << " (ms)" << endl;
	cout << "#graph vertexes: " << g.num_vertices() << "\t#graph edges: " << g.num_edges() << endl;

#if DEBUG

	ofstream out("./out");
	g.writeGraph(out);


#endif

	// Labeling happens here
	cout << "starting GRAIL labeling..." << endl;
	gettimeofday(&before_time, NULL);
#if THREADS
	Grail grail(std::ref(g), DIM, pool);
#endif
	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#Grail construction time: " << labeling_time << " (ms)" << endl;

	
	#if DEBUG
	print_graph(std::ref(g),"./label", DIM);
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
	search_reachability(std::ref(grail),std::ref(queries), std::ref(pool));

	gettimeofday(&after_time, NULL);
	query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#total query running time: " << query_time << " (ms)" << endl;
	ofstream queryout("./query");
	print_query(queryout,grail,queries);
	cout.setf(ios::fixed);		
	cout.precision(2);
	cout << "GRAIL REPORT " << endl;
	cout << "filename = " << filename << "\t testfilename = " << testfilename <<
			"\t DIM = " << DIM << endl;
	cout << "Labeling_time = " << labeling_time  << "\t Query_Time = " << query_time <<
			"\t Index_Size = " << g.num_vertices()*DIM*2  << endl;
}
