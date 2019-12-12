#pragma once

#define condi(x,y) (midCode.op2 == x && midCode.Type == y)

#define regt(x) -x
#define lowRegt 3
#define highRegt 9

#define regs(s) (-s-highRegt)
#define lowRegs 1
#define highRegs 7

#define rega(a) (-a-highRegt-highRegs)
#define lowRega 1
#define highRega 3

#define intensityBound 10

class localRegManager {
public:
	int currentBNo = -1;
	map<string, string> reg2Name;
	map<string, string> name2Reg;
	//count the reg remaining life
	map<string, int> regtLife;
	set<string> freeReg;
	set<string> poorSet;
	list<string> LRU;
	localRegManager();
	void newText();
	void free();
	void reset();
	void updateBlock(int no);
	int useVarMark(string name);
	string isVarDispatch(string name);
};


class globalRegManager {
public:
	map<string, map<string, string>> fun2Reg2Name;
	map<string, map<string, string>> fun2Name2Reg;
	map<string, map<string, int>> fun2Name2Remain;
	vector<string> regPool;
	vector<string> clashNameNow;
	set<string> usingRegPool;
	int intensityNow;

	void init(map<string, map<string, int>> countMap);
	bool isGlobalVar(string name, string funcNow);
	int useGlobalVarMark(string name, string funcNow);
	string getVarReg(string name, string funcNow);
	void setIntensity(int intensity);
	void jalFunc(string funcNow, string jalFunc);
	void raFunc(string funcNow, string jalFunc);
};

string getTempType(string name);

bool cmp(const pair<string, int>& a, const pair<string, int>& b);

void translate();

void transLi(string value, string& regName);

void printMiddleCode(middleCode mid);

void dotData_gen();

void diffFuncAndExit();

void dotText_gen();

void translate();

int dispatchReg(string name);

int searchData(string name, string& globalName, string& type);

void dispatch(string name, string type, string no);

string loadVarToReg(string name, string no, string& regName);

void storeRegToVar(string name, string no, string& regName);

void sentence(middleCode midCode, int index);

void printSentence(middleCode midCode);

void scanSentence(middleCode midCode);

void calSentence(middleCode midCode);

void loadArrSentence(middleCode midCode);

void storeArrSentence(middleCode midCode);

void callFuncSentence(middleCode midCode);

void pushParaSentence(middleCode midCode);

void conditionSentence(middleCode midCode);

void conditionJumpSentence(middleCode midCode);

void constDefineSentence(middleCode midCode);

void varDefineSentence(middleCode midCode);