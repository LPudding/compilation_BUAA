#pragma once
class BB {
public:
	int no;
	int firstLoc;//+1
	int lastLoc;//0
	string funcName;
	vector<int> nextBB;
	vector<int> prevBB;
	map<string, int> varCount;
	set<string> usedSet;
	set<string> useSet;
	set<string> defSet;
	set<string> lvIn;
	set<string> lvOut;

	
	BB(int num, int first, int last, string funcName);
};

#define constSpread 1

extern vector<BB> block;

extern set<string> globalBlockSet;
extern set<string> arraySet;

void liveVarLoop();

void pushSentenceHandle();

void tempVarHandle(bool& isOpt);

void varStatics(map<string, int>& varCount, string name);

//local or global
void varTypeAnalyse();

void deadVarHandle(bool& isOpt);

void liveVarAnalyse();

void simplifyExp(bool& isOpt);

void printBlock();

int getLabelBlockNo(string label);

int getIrBlockNo(int index);

void divideBlock();

void calVarUseDefSet();

void optimize();