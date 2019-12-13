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

#define constSpread 0

extern vector<BB> block;

extern map<string,set<string>> crossBlockTable;
extern set<string> globalSet;
extern set<string> arraySet;
extern map<string, map<string, int>> impVarCountMap;
extern set<string> recursiveFuncSet;

void change_intensity(middleCode mid, int& intensity);

void definedConstSpread();

void ImpVarCount();

void printImpVarCount();

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