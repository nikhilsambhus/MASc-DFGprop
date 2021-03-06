#include "DFGAnaly.h"
using namespace std;
DFGAnaly::DFGAnaly() {}
DFGAnaly::DFGAnaly(DAG grph) {
	gp = grph;
}
void DFGAnaly::topoSortHelper(uint32_t node, vector<bool> &visited, stack<uint32_t> &st) {
	visited[node] = true;
	list<Node> succs;
	gp.getSuccessors(node, succs);
	for(Node nd : succs) {
		uint32_t ndId = nd.getID();
		if(visited[ndId] == false) {
			topoSortHelper(ndId, visited, st);
		}
	}
	st.push(node);
}

vector<uint32_t> DFGAnaly::topoSort() {
	vector<uint32_t> order;
	stack<uint32_t> stNodes;

	vector<bool> visited;
	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		visited.push_back(false);
	}

	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		if(visited[i] == false) {
			topoSortHelper(i, visited, stNodes);
		}
	}

	while(!stNodes.empty()) {
		order.push_back(stNodes.top());
		stNodes.pop();
	}

	return order;
}
uint32_t DFGAnaly::assignTime(vector<uint32_t> &topOrder, vector<int32_t> &timeSt) {
	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		timeSt.push_back(-1);
	}

	int32_t timeMax = 0;
	int count_load_1 = 0;
	for(uint32_t n : topOrder) {
		//cout << n << " ";
		list<Node> preds;
		gp.getPredecessors(n, preds);
		int32_t max = 0;
		string label = gp.findNode(n)->getLabel();
		for(Node prNode : preds) {
			uint32_t id = prNode.getID();
			if(timeSt[id] == -1) {
				cout << "Error, predecessor time cannot be -1\n";
				exit(-1);
			}
			else if(max < timeSt[id]) {
				max = timeSt[id];
			}
		}
		/*if((label.find("load") != std::string::npos)) {
			int stride = stoi(label.substr(label.find(";") + 1, label.length()));
			if(stride >= STRIDE_MIN) {
				timeSt[n] = max;
			}
			else {
				timeSt[n] = max + 1;//nodeWts["load"];
			}
		}
		else if((label.find("store") != std::string::npos)) {
			int stride = stoi(label.substr(label.find(";") + 1, label.length()));
			if(stride >= STRIDE_MIN) {
				timeSt[n] = max;
			}
			else {
				timeSt[n] = max + 1; //nodeWts["store"];
			}
		}
		else {
			timeSt[n] = max + 1;//nodeWts[label];
		}*/
		timeSt[n] = max + 1;
		if(timeSt[n] == 1 && label.find("load") != std::string::npos) {
			count_load_1++;
		}

		if(timeMax < timeSt[n]) {
			timeMax = timeSt[n];
		}
	}

	cout << count_load_1 << " loads at timestamp 1 " << endl;
	return timeMax;
}
double DFGAnaly::getParallelism() {
	vector<uint32_t> topOrder = topoSort();
	vector<int32_t> timeSt; 
	uint32_t timeMax = assignTime(topOrder, timeSt);
	/*cout << "Time Max is " << timeMax << endl;
	cout << "Topological sort order is\n";
	for(uint32_t n : topOrder) {
		cout << gp.findNode(n)->getLabel() << " has time " << timeSt[n] << endl;
	}
	cout << endl;
	*/
	double total = 0;
	for(uint32_t i = 0; i <= timeMax; i++) {
		total+= count(timeSt.begin(), timeSt.end(), i);	
	}
	
	return total/timeMax;
}

uint32_t DFGAnaly::criticalPathLen() {
	vector<uint32_t> topOrder = topoSort();
	vector<int32_t> timeSt; 
	uint32_t timeMax = assignTime(topOrder, timeSt);
	for(int i = 0; i <= timeMax; i++) {
		cout << " Node level " << i << " has count " << count(timeSt.begin(), timeSt.end(), i)  << " " ;
	}
	cout << endl;
	return timeMax;
}

void DFGAnaly::getBasicProps() {
	uint32_t nodes = gp.getNumNodes();
	uint32_t edges = gp.getNumEdges();
	double deg = (double) edges / (double) nodes;
	cout << "Nodes " << nodes << " Edges " << edges << " Degree " << deg << endl;

	int cnt, fmax, fmin;
	double favg;

	//fin
	cnt = 0;
	favg = 0;
	fmax = INT_MIN;
	fmin = INT_MAX;
	for(uint32_t i = 0; i < nodes; i++) {
		string label = gp.findNode(i)->getLabel();
		if((label.find("load") != std::string::npos)) {
			continue; //skip load nodes for fin
		}
		cnt++;
		list<Edge> ins;
		gp.getInEdges(i, ins);
		if(fmax < int(ins.size())) {
			fmax = ins.size();
		}

		if(fmin > ins.size()) {
			fmin = ins.size();
		}
		favg += ins.size(); 
	}
	favg = favg/cnt;
	cout << "Fin stats:- Max: " << fmax << " Min: " << fmin << " Avg: " << favg << endl;

	//fout
	cnt = 0;
	favg = 0;
	fmax = INT_MIN;
	fmin = INT_MAX;
	for(int i = 0; i < nodes; i++) {
		string label = gp.findNode(i)->getLabel();
		if((label.find("store") != std::string::npos)) {
			continue; //skip store nodes for fout
		}
		cnt++;
		list<Edge> outs;
		gp.getOutEdges(i, outs);
		if(fmax < int(outs.size())) {
			fmax = outs.size();
		}

		if(fmin > outs.size()) {
			fmin = outs.size();
		}
		favg += outs.size(); 
	}
	favg = favg/cnt;
	cout << "Fout stats:- Max: " << fmax << " Min: " << fmin << " Avg: " << favg << endl;
}

