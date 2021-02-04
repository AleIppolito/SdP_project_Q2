
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
 **************************************
 *
 * This reads the queries file
 */

void read_test(std::vector<query> &q) {
	int s,t,label=0;
	ifstream fstr(testfilename);
	if(isquer)
		while(fstr >> s >> t)
			q.push_back({s,t,label});
	else
		while(fstr >> s >> t >> label)
			q.push_back({s,t,label});
}

#if DEBUG
void print_test(std::vector<query> queries) {
	ofstream outfile("../../project_generator/graphGenerator-StQ/outfile.que");
	for (auto &q : queries) {
		outfile <<  q.src << " " << q.trg << endl;
	}
}
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

/*
 * **********************************
 */

int main(int argc, char* argv[]) {
#if THREADS

	ThreadPool pool(std::thread::hardware_concurrency());

#endif
	parse_args(argc,argv);
	/*
	 *	Read Graph from the input file AND prepare queries
	 */

	srand48(time(NULL));
	struct timeval after_time, before_time, after_timepart;
	gettimeofday(&before_time, NULL);

	std::vector<query> queries;

#if THREADS
	// Graph reading thread returns the graph as a promise
	std::promise<Graph> pgraph;
	std::future<Graph> fgraph = pgraph.get_future();
	std::thread readGraphThread(read_graph,std::ref(pgraph));
	std::thread testfileThread(&read_test, std::ref(queries));


	// While the graph is running run a thread that reads the test file
	Graph g = fgraph.get();
	readGraphThread.join();
	testfileThread.join();
#else

	Graph g(filename);

	read_test(queries);

#endif

	/*
	 * Time check evaluation
	 */

	gettimeofday(&after_time, NULL);
	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#graph read time: " << labeling_time << " (ms)" << endl;
	cout << "#vertex size: " << g.num_vertices() << "\t#edges size: " << g.num_edges() << endl;

	ofstream out("./out");
	g.writeGraph(out);
	/*
	 * Labeling happens here
	 */
	cout << "Starting GRAIL labeling... " << endl;
	gettimeofday(&before_time, NULL);

	Grail grail(g, DIM);

	gettimeofday(&after_time, NULL);

	labeling_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
			(after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
	cout << "#Grail construction time: " << labeling_time << " (ms)" << endl;

	/*
	 * Query processing happens here
	 */

	cout << "Starting reachability testing..." << endl;
	gettimeofday(&before_time, NULL);

	int source, target;
	int reachable = 0, nonreachable =0;
	int success = 0, fail = 0;

	bool r;
	/*
	 * Let's put the false positives and negatives in respective files,
	 * For this I created a 'Reports' folder
	 */

	ofstream falsepos("../Reports/falsepositives.que");
	ofstream falseneg("../Reports/falsenegatives.que");

	/*
	 * Here we use auto, but let's just make sure Savino likes this, otherwise we'll use a vector iterator
	 */
	for (auto &qit : queries) {
		r = grail.bidirectionalReach(qit.src, qit.trg);
		if(isquer){
			(r==true) ? reachable++ : nonreachable++;
		} else {
			if(r==true) {
				reachable++;
				success++;
				if(qit.labels == 0) {
					falsepos << "False positive pair = " << qit.src << " " << qit.trg << " " << qit.labels << endl;
					falsepos << "Levels : " << qit.src  << "->" << g[qit.src ].top_level << " " << qit.trg << "->" << g[qit.trg].top_level << endl;
					fail++;
					success--;
				}
			} else {
				nonreachable++;
				success++;
				if(qit.labels == 1) {
					falseneg << "False negative pair = " << qit.src  << " " << qit.trg << " " << qit.labels << endl;
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
			"\t Index_Size = " << g.num_vertices()*DIM*2  << endl;
}
