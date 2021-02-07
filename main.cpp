
/*
 * Copyright (c) Hilmi Yildirim 2010,2011.
 * Changes made on his code, available on Git
 */

#include "Grail.h"

bool isquer = false;
float graph_time, labeling_time, query_time;

struct query{
	int src;
	int trg;
	int labels;
};

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

static void parse_args(int argc, char *argv[], string &fname, string &tfname, int &dim) {
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

// Read testfilename and prepare queries vector
void read_test(const string &tfname, std::vector<query> &q) {
#if DEBUG
	cout << __func__ << endl;
#endif
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

void read_graph(const string &fname, Graph &gr) {
#if DEBUG
	cout << __func__ << endl;
#endif
	gr.readGraph(fname);
}

void print_query(ostream & out,Grail& grail, std::vector<query>& queries) {
	int i = 0;
	cout << "Queries solution : " << endl;
	for (auto &q : queries)
		out << q.src << " " << q.trg << " " << grail.getReachability(i++) << endl;
}

void Wbidirectional(Grail &grail, int src, int trg, int query_id) {
	grail.bidirectionalReach(src,trg,query_id);
}

void search_reachability(Grail& grail, const std::vector<query> &queries, ThreadPool &pool){
	grail.setReachabilty(queries.size());
	int i =0;
	for (auto &q : queries) {
		pool.addJob(Wbidirectional, std::ref(grail),q.src, q.trg, i);
		i++;
	}
	pool.waitFinished();
}
// --------------------------------------------------------------------

int main(int argc, char* argv[]) {
	string filename, testfilename;
	int DIM = 2;
	parse_args(argc, argv, filename, testfilename, DIM);
#if THREADS
	ThreadPool pool;	// standard constructor, based on std::thread::hardware_concurrency()
#endif
	// Read Graph from the input file AND prepare queries
	std::vector<query> queries;
	srand48(time(NULL));	// random init
	struct timeval before_time, after_time;
	cout << "processing input files..." << endl;
	gettimeofday(&before_time, NULL);

#if THREADS
	pool.addJob(read_test, std::ref(testfilename), std::ref(queries));
	Graph g;
	pool.addJob(read_graph, std::ref(filename), std::ref(g));
	pool.waitFinished();
#else	// no THREADS
	read_test(testfilename, queries);
	Graph g;
	read_graph(filename, g);
#endif
	gettimeofday(&after_time, NULL);
#if DEBUG
	ofstream qout("./queries");
	cout << "print_queries_and_nodes" << endl;
	for(query &q: queries)
		qout << q.src << " " << q.trg << " " << q.labels << endl;
	qout.close();

	ofstream gout("./graph");
	g.writeGraph(gout);
#endif
	// Time check evaluation
	graph_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#input files read time: " << graph_time << " (ms)" << endl;
	cout << "#graph vertexes: " << g.num_vertices() << "\t#graph edges: " << g.num_edges() << endl;

	// Labeling happens here
	cout << "\nstarting GRAIL labeling..." << endl;
	gettimeofday(&before_time, NULL);
#if THREADS
	Grail grail(std::ref(g), DIM, pool);
#else
	Grail grail(std::ref(g), DIM);
#endif
	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#GRAIL construction time: " << labeling_time << " (ms)" << endl;

	// Query processing happens here

	cout << "\nstarting reachability testing..." << endl;
	gettimeofday(&before_time, NULL);

	search_reachability(std::ref(grail),std::ref(queries), std::ref(pool));

	gettimeofday(&after_time, NULL);
	query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 + 
		(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#total query running time: " << query_time << " (ms)" << endl;
	//ofstream queryout("./query");
	//print_query(queryout,grail,queries);
	cout.setf(ios::fixed);		
	cout.precision(2);
	cout << "GRAIL REPORT " << endl;
	cout << "filename: " << filename << " testfilename: " << testfilename << " traversals: " << DIM << endl;
	cout << "graph construction = " << graph_time << "\nlabeling time = " << labeling_time  << "\nquery time = "
			<< query_time << endl;// << "\nindex_size = " << g.num_vertices()*DIM*2  << endl;
	//cout << "all done\n\nSuccess Rate " << success << "/" << success+fail << endl;
}
