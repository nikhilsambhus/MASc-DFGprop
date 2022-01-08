#include "DFGUtils.h"
#include <vector>
#include <stack>
#include <list>
#include <bits/stdc++.h>
#include <string>

void topoSortHelper(DAG &gp, uint32_t node, vector<bool> &visited, stack<uint32_t> &st) {
	visited[node] = true;
	list<Node> succs;
	gp.getSuccessors(node, succs);
	for(Node nd : succs) {
		uint32_t ndId = nd.getID();
		if(visited[ndId] == false) {
			topoSortHelper(gp, ndId, visited, st);
		}
	}
	st.push(node);
}

vector<uint32_t> topoSort(DAG &gp) {
	vector<uint32_t> order;
	stack<uint32_t> stNodes;

	vector<bool> visited;
	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		visited.push_back(false);
	}

	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		if(visited[i] == false) {
			topoSortHelper(gp, i, visited, stNodes);
		}
	}

	while(!stNodes.empty()) {
		order.push_back(stNodes.top());
		stNodes.pop();
	}

	return order;
}
uint32_t assignTime(DAG &gp, vector<uint32_t> &topOrder, vector<uint32_t> &timeSt) {
	for(uint32_t i = 0; i < gp.getNumNodes(); i++) {
		timeSt.push_back(-1);
	}

	uint32_t timeMax = 0;
	for(uint32_t n : topOrder) {
		list<Node> preds;
		gp.getPredecessors(n, preds);
		uint32_t max = 0;
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
		if((label.find("load") != std::string::npos)) {
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
		}

		if(timeMax < timeSt[n]) {
			timeMax = timeSt[n];
		}
	}

	return timeMax;
}
double getParallelism(DAG &gp) {
	vector<uint32_t> topOrder = topoSort(gp);
	vector<uint32_t> timeSt; 
	uint32_t timeMax = assignTime(gp, topOrder, timeSt);
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

uint32_t criticalPathLen(DAG &gp) {
	vector<uint32_t> topOrder = topoSort(gp);
	vector<uint32_t> timeSt; 
	uint32_t timeMax = assignTime(gp, topOrder, timeSt);
	return timeMax;
}

void getBasicProps(DAG &gp) {
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
	for(int i = 0; i < nodes; i++) {
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

vector<vector<int>> getCombs(int timeMax, int k) {
	int n = timeMax;
	std::string bitmask(k, 1); // K leading 1's
	bitmask.resize(n, 0); // N-K trailing 0's

	vector<vector<int>> combs;
	do {
		vector<int> splits;
		for (int i = 0; i < n; ++i) 
		{
			if (bitmask[i]) { 
				//std::cout << " " << i + 1;
				splits.push_back(i);
			}
		}
		combs.push_back(splits);
		//std::cout << std::endl;
	} while (std::prev_permutation(bitmask.begin(), bitmask.end()));
	
	return combs;
}

partData getInterNds(int start, int end, DAG &gp, vector<uint32_t> &timeSt) {
	uint32_t total = 0, out = 0;
	partData pData;
	for(uint32_t nd = 0; nd < timeSt.size(); nd++) {
		if(timeSt[nd] >= start && timeSt[nd] <= end) {
			list<Node> succs;
			string label = gp.findNode(nd)->getLabel();
			total++;
			gp.getSuccessors(nd, succs);
			bool outYes = false;
			for(Node nd : succs) {
				if(timeSt[nd.getID()] > end) {
					outYes = true;
					break;
				}
			}
			if(outYes == true) {
				out++;
			}
		}
	}
	pData.total = total;
	pData.out = out;
	return pData;
}

int partitionDFGnP(DAG &gp, int npart, int map_size) {
	vector<uint32_t> topOrder = topoSort(gp);
	vector<uint32_t> timeSt; 
	int applicableParts = 0;
	uint32_t timeMax = assignTime(gp, topOrder, timeSt);
	vector<vector<int>> combs = getCombs(timeMax, npart - 1);
	for(vector<int> splits: combs) {
		int prev = 0;
		vector<partDef> AllParts;
		splits.push_back(timeMax);
		for(int split : splits) {
			partData pData = getInterNds(prev, split, gp, timeSt);
			if(pData.total > map_size) {
				break;
			}
			//cout << " Pno. " << pno << "(" << prev << "-" << split << ") Total nodes: " << pData.total << " Intermediate outputs: " << pData.out; 
			partDef pd;
			pd.start = prev;
			pd.end = split;
			pd.pData = pData;
			prev = split + 1;
			AllParts.push_back(pd);
		}
		//partData pData = getInterNds(prev, timeMax, gp, timeSt);
		//cout << " Pno. " << pno << "(" << prev << "-" << timeMax << ") Total nodes: " << pData.total << " Intermediate outputs: " << pData.out; 
		if(AllParts.size() == npart) {
			int pno = 1;
			for(auto pd: AllParts) {
				cout << " Pno. " << pno << "(" << pd.start << "-" << pd.end << ") Total nodes: " << pd.pData.total << " Intermediate outputs: " << pd.pData.out; 
				pno++;
			}
			applicableParts++;
			cout << endl;
		}
	}
	return applicableParts;
}

void partitionDFGVar(DAG &graph, int map_size) {
	for(int nparts = 2; nparts <= 4; nparts++) {
		cout << "Trying with " << nparts << endl;
		if(partitionDFGnP(graph, nparts, map_size)) {
			cout << "Success\n" << endl;
			break;
		}
	}
}

void partitionDFG(DAG &gp) {
	vector<uint32_t> topOrder = topoSort(gp);
	vector<uint32_t> timeSt; 
	uint32_t timeMax = assignTime(gp, topOrder, timeSt);
	uint32_t part1 = 0;

	for(uint32_t step = 0; step <= timeMax; step++) {
		uint32_t nds = std::count(timeSt.begin(), timeSt.end(), step);
		part1 += nds;
		partData pData = getInterNds(0, step, gp, timeSt);
		cout << "If split at " << step  << " Partition sizes " << part1 << " & " << timeSt.size() - part1 << " intermediate load/stores " << pData.out << endl;
	}
	
}
int main(int argc, char **argv) {
	string fname = argv[1];
	DAG graph(fname);

	double par = getParallelism(graph);
	cout << "Parallelism in graph is " << par << endl;
	uint32_t clen = criticalPathLen(graph);
	cout << "Critical path length is " << clen << endl;
	
	getBasicProps(graph);

	//partitionDFG(graph);
	int cgra_size = atoi(argv[2]);
	int routing_size = atoi(argv[3]);
	int map_size = cgra_size - routing_size;
	partitionDFGVar(graph, map_size);
	return 0;
}
