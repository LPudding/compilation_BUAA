#include "includeH.h"
//middle code


ofstream mips_code;

vector<string> stringTable;//string array
const string stringName = "_string_";
const string varName = "_var_";
const string arrayName = "_array_";

//before:type name value      length
//after: type name globalName length  store the global var and array
vector<middleCode> globalTable;

//type name length 
vector<middleCode> stackTable;
int stackIndex = 0;//current stack index
int lastIndex;

//type name index num
vector<middleCode> paraList;

localRegManager LocalManager;
globalRegManager globalManager;

string funcNow;

string loadStoreType = "";//""->default "memory"->memory "NOTGLOBAL"
//string calIntensity = "";//""->cal "ls"->load and store

void dotData_gen() {
	bool isGlobal = true;
	bool isMain = false;
	//handle var and const(global) str
	for (vector<middleCode>::const_iterator iter = middleTable.table.begin(); iter != middleTable.table.end(); ++iter) {
		if (iter->Type == "FUNCHEAD") {
			isGlobal = false;
		}
		if (iter->Type == "MAINHEAD") {
			isGlobal = false;
			isMain = true;
		}
		if (isGlobal && iter->Type == "VAR") {
			globalTable.push_back(middleCode(iter->op2, iter->op1, "", iter->op3));
		}
		if (isGlobal && iter->Type == "CONST") {
			globalTable.push_back(middleCode(iter->op2, iter->op1, iter->op3, "0"));
		}
		//store main str first
		if (isMain && iter->Type == "PRINT" && iter->op2 == "0") {//str
			stringTable.push_back(iter->op1);
		}
	}
	//store func str second
	isMain = false;
	for (vector<middleCode>::const_iterator iter = middleTable.table.begin(); iter != middleTable.table.end(); ++iter) {
		if (iter->Type == "MAINHEAD") {
			isMain = true;
		}
		if (!isMain && iter->Type == "PRINT" && iter->op2 == "0") {//str
			stringTable.push_back(iter->op1);
		}
	}
	mips_code << ".data" << endl;
	//string
	for (unsigned int i = 0; i < stringTable.size(); i++) {
		mips_code << stringName + to_string(i) + " : .asciiz \"" + stringTable[i] + "\"" << endl;
		stringTable[i] = stringName + to_string(i);
	}
	mips_code << stringName + " : .asciiz \"\\n\"" << endl;
	//var and array
	for (unsigned int i = 0, j = 0, k = 0; i < globalTable.size() * 2; i++) {
		int length = stoi(globalTable[i % globalTable.size()].op3);
		//handle var first
		if (length == 0 && i < globalTable.size()) {
			string type;
			if (globalTable[i].op2 == "") {
				type = (globalTable[i].Type == "INT") ? "word 0" : "byte 0";
			}
			else {
				if (globalTable[i].Type == "INT") {
					type = "word " + globalTable[i].op2;
				}
				else {
					type = "byte " + globalTable[i].op2;
				}
			} 
			string name = varName + to_string(j++);
			globalTable[i].op2 = name;
			mips_code << name + " : ." + type << endl;
		}
		//handle array second
		else if (length > 0 && i >= globalTable.size()){
			string size = to_string(stoi(globalTable[i % globalTable.size()].op3));
			string name = arrayName + to_string(k++);
			globalTable[i % globalTable.size()].op2 = name;
			mips_code <<  name + " : .word 0:" + size << endl;
		}
	}

}

localRegManager::localRegManager() {
	reset();
}

void localRegManager::newText() {
	currentBNo = -1;
}

void localRegManager::reset() {
	//reset freeReg
	freeReg.clear();
	size_t i = lowRegt;
	for (; i <= highRegt; i++) {
		freeReg.insert("$t" + to_string(i));
	}
	//reset map
	reg2Name.clear();
	name2Reg.clear();
	//reset poorSet
	poorSet.clear();
}

void localRegManager::free() {
	/*for (map<string, string>::iterator it = reg2Name.begin(); it != reg2Name.end(); it++) {
		storeRegToVar(it->second, "0", it->first);
	}*/
	reset();
}

void localRegManager::updateBlock(int no) {
	if (currentBNo == -1) {
		currentBNo = no;
	}
	else if (currentBNo != no) {
		currentBNo = no;
		free();
	}
}

int localRegManager::useVarMark(string name) {
	/*if (poorSet.find(name) == poorSet.end() && name2Reg.find(name) == name2Reg.end()) {
		dispatch(name, getTempType(name), "0");
	}*/
	if (!name2Reg.empty() && name2Reg.find(name) != name2Reg.end()) {
		//has been dispatched
		string reg = name2Reg[name];
		regtLife[name]--;

		if (regtLife[name]) {
			LRU.remove(reg);
			LRU.push_front(reg);
		}
		else {
			LRU.remove(reg);
			freeReg.insert(reg);
			name2Reg.erase(name2Reg.find(name));
			reg2Name.erase(reg2Name.find(reg));
			cout << "free " + reg << endl;

		}
		return -stoi(reg.substr(2));
	}
	else {
		string reg;
		// waiting to be dispatched
		if (!freeReg.empty()) {
			//have free reg now
			reg = *freeReg.begin();
			cout << "dispatch new " << reg << " to " << name << endl;
			//freeReg
			freeReg.erase(freeReg.begin());
			//map
			name2Reg.insert(make_pair(name, reg));
			reg2Name.insert(make_pair(reg, name));
			//LRU
			LRU.push_front(reg);
			//regt life
			regtLife.insert(make_pair(name, block[currentBNo].varCount[name] - 1));
		}
		else {
			//LRU
			reg = LRU.back();
			LRU.pop_back();
			LRU.push_front(reg);
			//save poor var
			string poorName = reg2Name[reg];
			poorSet.insert(poorName);
			block[currentBNo].varCount[poorName] = regtLife[poorName];

			loadStoreType = "memory";
			storeRegToVar(poorName, "0", reg);
			loadStoreType = "";

			cout << "dispatch new " << reg << " to " << name << endl;
			reg2Name.erase(reg2Name.find(reg));
			name2Reg.erase(name2Reg.find(poorName));
			regtLife.erase(regtLife.find(poorName));
			//new var(may be a poor var)
			if (poorSet.find(name) != poorSet.end()) {
				loadStoreType = "memory";
				string warn = loadVarToReg(name, "0", reg);
				loadStoreType = "";
			}
			//regt life
			regtLife.insert(make_pair(name, block[currentBNo].varCount[name]));
			reg2Name.insert(make_pair(reg, name));
			name2Reg.insert(make_pair(name, reg));
		}
		return -stoi(reg.substr(2));
	}
}

string localRegManager::isVarDispatch(string name) {
	string globalName, type;
	loadStoreType = "NOTGLOBAL";
	int index = searchData(name,globalName,type);
	loadStoreType = "";
	if (index == -2) {
		dispatch(name, getTempType(name), "0");
		index = searchData(name, globalName, type);
	}
	//cout << "index:" + to_string(index) << endl;
	if (index >= 0) {
		//cout << loadStoreType << endl;
		//cout << to_string(arraySet.find(name) == arraySet.end()) << endl;
		//cout << to_string(globalBlockSet.find(name) == globalBlockSet.end()) << endl;
	}
	if (regt(index) >= lowRegt && regt(index) <= highRegt) {
		return "$t" + to_string(regt(index));
	}
	return "NOFOUND";
}

int dispatchReg(string name) {
	return 0;//no free reg for now
}



/*************************************************************************************
	if data in local stack return the relative position and type    return value    >=  0;
	if data in global table return the global name and type         return value     = -1;
	$t3-t9                                                          return value -2 -> -9;
	$s1-s7															return value -10->-16;
	$a1-a3															return value -17->-19;0
	if can't find data                                              return value     = -2;
	************************************************************************************/

int searchData(string name,string& globalName,string& type) {
	int count = 0;
	for (unsigned int i = stackIndex; i < stackTable.size(); i++) {
		//mips_code << "name:" + stackTable[i].op1 << endl;
		if (stackTable[i].op1 == name) {
			type = stackTable[i].Type;
			set<string> crossSet = crossBlockTable[funcNow];
			if ((crossSet.empty() || (crossSet.find(name) == crossSet.end()))&&(arraySet.empty() || (arraySet.find(name) == arraySet.end()))) {
				// temp var lived in block 
				if (loadStoreType != "memory") {
					cout << "#######cache t " + name << endl;
					mips_code << "#######cache t " + name << endl;
					return LocalManager.useVarMark(name);
				}	
			}
			//cout << "find:" << name << " inFunc:" << funcNow << endl;
			if (globalManager.isGlobalVar(name, funcNow)) {
				if (loadStoreType != "memory" && loadStoreType != "NOTGLOBAL") {
					cout << "#######cache s/a " + name << endl;
					mips_code << "#######cache s/a " + name << endl;
					return globalManager.useGlobalVarMark(name, funcNow);
				}
			}
			return (count + i - stackIndex) << 2;
		}
		count += stoi(stackTable[i].op2);
	}
	for (unsigned int i = 0; i < globalTable.size(); i++) {
		if (globalTable[i].op1 == name) {
			type = globalTable[i].Type;
			globalName = globalTable[i].op2;
			return -1;
		}
	}
	return -2;
}

void dispatch(string name,string type,string no) {
	if (dispatchReg(name) == 0) {
		stackTable.push_back(middleCode(type, name, no, ""));
		mips_code << "#					define:" + name + " type:" + type << endl;
		cout << "#  define:" + name << endl;
	}
}

string getTempType(string name) {
	string head = name.substr(0, 3);
	if (head == "INT")
		return head;
	head = name.substr(0, 4);
	if (head == "CHAR")
		return head;
	return "VOID";
}

//no can be a num or a var
//name can be a num or a char or a var
string loadVarToReg(string name, string no, string& regName) {
	mips_code << "				# load " + name + "->" + regName << endl;
	int val;
	if (isNumStr(name,val)) {
		transLi(name,regName);
		return "INT";
	}
	bool neg = false;
	if (name[0] == '-') {
		neg = true;
		name = name.substr(1);
	}
	if (name == "RET") {
		regName = "$v0";
		if (neg) {
			mips_code << "    sub $v0 $0 $v0" << endl;
		}
		return "INT";
	}
	if (name[0] == '\'') {
		if (neg)
			mips_code << "    subi " + regName + " $0 " + name << endl;
		else
			transLi(name, regName);
		return "CHAR";
	}
	/*if (name == "RET") {
		mips_code << "    move " + regName + " $v0" << endl;
		if (neg)
			mips_code << "    sub " + regName + " $0 " + regName << endl;
		return "INT";
	}*/
	string globalName, type;
	int index = searchData(name, globalName, type);
	if (index == -2) {
		dispatch(name, getTempType(name), "0");
		index = searchData(name, globalName, type);
	}
	mips_code << "				# offset: " + to_string(index) + " no: " + no << endl;
	assert(index != -2);
	if (index >= 0) {
		int val;
		if (isNumStr(no, val)) {
			mips_code << "    lw " + regName + " -" + to_string(index + stoi(no) * 4) + "($sp)" << endl;
		}
		else {
			string reg = "$t1";
			string warn = loadVarToReg(no, "0", reg);
			if (reg != "$t1") {
				mips_code << "    move $t1 " + reg << endl;
			}
			mips_code << "    sll $t1 $t1 2" << endl;
			mips_code << "    li $t2 " + to_string(index) << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			mips_code << "    sub $t1 $sp $t1" << endl;
			mips_code << "    lw " + regName + " 0($t1)" << endl;
		}

	}
	else if (index == -1) {
		mips_code << "    la " << "$t1 " + globalName << endl;
		int val;
		if (isNumStr(no, val)) {
			if (type == "INT")
				mips_code << "    lw " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
			else
				mips_code << "    lb " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
		}
		else {
			string reg = "$t2";
			string warn = loadVarToReg(no, "0", reg);
			if (reg != "$t2") {
				mips_code << "    move $t2 " + reg << endl;
			}
			mips_code << "    sll $t2 $t2 2" << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			if (type == "INT")
				mips_code << "    lw " + regName + " 0($t1)" << endl;
			else
				mips_code << "    lb " + regName + " 0($t1)" << endl;
		}
	}
	else if (regt(index) >= lowRegt && regt(index) <= highRegt) {
		regName = "$t" + to_string(regt(index));
	}
	else if (regs(index) >= lowRegs && regs(index) <= highRegs) {
		regName = "$s" + to_string(regs(index));
	}
	else if (rega(index) >= lowRega && rega(index) <= highRega) {
		regName = "$a" + to_string(rega(index));
	}
	//question
	if (neg) {
		mips_code << "    sub " << "$t0" << " $0 " + regName << endl;
		regName = "$t0";
	}
	return type;
}

//name:varName
//name->regName
void storeRegToVar(string name, string no,string& regName) {
	string globalName, type;
	int index = searchData(name, globalName, type);
	mips_code << "				# store " + regName + "->" + name << endl;
	if (index == -2) {
		dispatch(name, getTempType(name), "0");
		index = searchData(name, globalName, type);
	}
	if (index >= 0) {
		int val;
		if (isNumStr(no,val)) {
			mips_code << "    sw " + regName + " -" + to_string(index + stoi(no) * 4) + "($sp)" << endl;
		}
		else {
			string reg = "$t1";
			string warn = loadVarToReg(no, "0", reg);
			if (reg != "$t1") {
				mips_code << "    move $t1 " + reg << endl;
			}
			mips_code << "    sll $t1 $t1 2" << endl;
			mips_code << "    li $t2 " + to_string(index) << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			mips_code << "    sub $t1 $sp $t1" << endl;
			mips_code << "    sw " + regName + " 0($t1)" << endl;
		}
	}
	else if (index == -1) {
		mips_code << "    la " << "$t1 " + globalName << endl;
		int val;
		if (isNumStr(no,val)) {
			if (type == "INT")
				mips_code << "    sw " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
			else
				mips_code << "    sb " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
		}
		else {
			string reg = "$t2";
			string warn = loadVarToReg(no, "0", reg);
			if (reg != "$t2") {
				mips_code << "    move $t2 " + reg << endl;
			}
			mips_code << "    sll $t2 $t2 2" << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			if (type == "INT")
				mips_code << "    sw " + regName + " 0($t1)" << endl;
			else
				mips_code << "    sb " + regName + " 0($t1)" << endl;
		}
	}
	else if ((regt(index) >= lowRegt) && (regt(index) <= highRegt)) {
		regName = "$t" + to_string(regt(index));
	}
	else if (regs(index) >= lowRegs && regs(index) <= highRegs) {
		regName = "$s" + to_string(regs(index));
	}
	else if (rega(index) >= lowRega && rega(index) <= highRega) {
		regName = "$a" + to_string(rega(index));
	}
}

void printSentence(middleCode midCode) {
	if (midCode.op2 == "0") {
		//str
		mips_code << "# print " + stringTable[0] << endl;
		mips_code << "    li $v0 4" << endl;
		mips_code << "    la $a0 " + stringTable[0] << endl;
		mips_code << "    syscall" << endl;
		vector<string>::iterator k = stringTable.begin();
		stringTable.erase(k);
	}
	else if (midCode.op2 == "1") {
		// 1:iden
		// 2:"'s'"
		// 3:"12340"
		mips_code << "# print " + midCode.op1 << endl;
		string regName = "$a0";

		string type = loadVarToReg(midCode.op1, "0", regName);
		if (regName != "$a0") {
			mips_code << "    move $a0 " + regName << endl;
		}
		if (regName == "$0") {
			mips_code << "    add $a0 $0 $0" << endl;
		}
		if (type == "INT") {
			mips_code << "    li $v0 1" << endl;
		}
		else if (type == "CHAR") {
			mips_code << "    li $v0 11" << endl;
		}
		mips_code << "    syscall" << endl;
	}
	if (midCode.op3 != "0") {
		mips_code << "# print \\n" << endl;
		mips_code << "    li $v0 4" << endl;
		mips_code << "    la $a0 " + stringName << endl;
		mips_code << "    syscall" << endl;
	}
}

void scanSentence(middleCode midCode) {
	string globalName, type;
	int index = searchData(midCode.op1, globalName, type);
	mips_code << "# read " + midCode.op1 << endl;
	if (type == "INT") {
		mips_code << "    li $v0 5" << endl;
	}
	else if (type == "CHAR") {
		mips_code << "    li $v0 12" << endl;
	}
	mips_code << "    syscall" << endl;
	if (index >= 0) {
		mips_code << "    sw $v0 -" + to_string(index) + "($sp)" << endl;
	}
	else if (index == -1){
		mips_code << "    la $t0 " + globalName << endl;
		mips_code << "    sw $v0 0($t0)";
	}
	else if (regt(index) >= lowRegt && regt(index) <= highRegt) {
		string reg = "$t" + to_string(regt(index));
		mips_code << "    move " + reg + " $v0";
	}
	else if (regs(index) >= lowRegs && regs(index) <= highRegs) {
		string reg = "$s" + to_string(regs(index));
		mips_code << "    move " + reg + " $v0";
	}
	else if (rega(index) >= lowRega && rega(index) <= highRega) {
		string reg = "$a" + to_string(rega(index));
		mips_code << "    move " + reg + " $v0";
	}
	
}

void calSentence(middleCode midCode) {
	string reg1 = "$t0", reg2 = "$t1", reg3 = "$t0", globalName, type;
	mips_code << "# " + midCode.Type + " " + midCode.op2 + " " + midCode.op3 + "->" + midCode.op1 << endl;
	cout << "# " + midCode.Type + " " + midCode.op2 + " " + midCode.op3 + "->" + midCode.op1 << endl;

	int val1, val2;
	bool isConst1, isConst2;
	isConst1 = isNumStr(midCode.op2, val1);
	isConst2 = isNumStr(midCode.op3, val2);	

	//local
	string name1 = LocalManager.isVarDispatch(midCode.op1);
	mips_code << "#cal:op1=" << name1 << endl;
	if (name1 != "NOFOUND") {
		reg3 = name1;
	}
	//global
	cout << reg3 << endl;
	name1 = globalManager.getVarReg(midCode.op1,funcNow);
	if (name1 != "NOFOUND") {
		reg3 = name1;
	}
	cout << reg3 << endl;
	if (midCode.Type == "ADD") {
		if (isConst1 && isConst2) {
			mips_code << "    addi " + reg3 + " $0 " + to_string(val1) << endl;
		}
		else if (isConst1 && !isConst2) {
			string warn = loadVarToReg(midCode.op3, "0", reg2);
			mips_code << "    addi " + reg3 + " " + reg2 + " " + to_string(val1) << endl;
		}
		else if (!isConst1 && isConst2) {
			string warn = loadVarToReg(midCode.op2, "0", reg1);
			mips_code << "    addi " + reg3 + " " + reg1 + " " + to_string(val2) << endl;
		}
		else {
			if (midCode.op2[0] == '-') {
				string warn = loadVarToReg(midCode.op3, "0", reg1);
				warn = loadVarToReg(midCode.op2.substr(1), "0", reg2);
				mips_code << "    sub " + reg3 + " " + reg1 + " " + reg2 << endl;
			}
			else {
				string warn = loadVarToReg(midCode.op2, "0", reg1);
				warn = loadVarToReg(midCode.op3, "0", reg2);
				mips_code << "    add " + reg3 + " " + reg1 + " " + reg2 << endl;
			}

		}
	}
	else if (midCode.Type == "SUB") {
		//SUB v v c
		string warn;
		int val1, val2;
		bool isConst1, isConst2;
		isConst1 = isNumStr(midCode.op2, val1);
		isConst2 = isNumStr(midCode.op3, val2);
		if (isConst1 && isConst2) {//SUB v c c
			mips_code << "    addi " + reg3 + " $0 " + to_string(val1 - val2) << endl;
		}
		else if (!isConst1 && isConst2) {//SUB v v c
			warn = loadVarToReg(midCode.op2, "0", reg1);
			mips_code << "    addi " + reg3 + " " + reg1 + " " + to_string(-val2) << endl;
		}
		else {
			warn = loadVarToReg(midCode.op2, "0", reg1);
			warn = loadVarToReg(midCode.op3, "0", reg2);
			mips_code << "    sub " + reg3 + " " + reg1 + " " + reg2 << endl;
		}

	}
	else if (midCode.Type == "MUL") {
		string warn = loadVarToReg(midCode.op2, "0", reg1);
		warn = loadVarToReg(midCode.op3, "0", reg2);
		mips_code << "    mul " + reg3 + " " + reg1 + " " + reg2 << endl;
	}
	else if (midCode.Type == "DIV") {
		string warn = loadVarToReg(midCode.op2, "0", reg1);
		warn = loadVarToReg(midCode.op3, "0", reg2);
		mips_code << "    div " + reg1 + " " + reg2 << endl;
		mips_code << "    mflo " + reg3 << endl;
	}
	if (reg3 == "$t0")
		storeRegToVar(midCode.op1, "0", reg3);
}

void loadArrSentence(middleCode midCode) {
	mips_code << "# load_arr " + midCode.op2 + "[" + midCode.op3 + "]" + " -> " + midCode.op1 << endl;
	string valueReg = "$t1";
	string warn = loadVarToReg(midCode.op2, midCode.op3, valueReg);
	string globalName, type;
	if (getTempType(midCode.op1) != "VOID") {
		searchData(midCode.op2, globalName, type);//get tempVar type
		dispatch(midCode.op1, type,"0");
	}
	storeRegToVar(midCode.op1, "0", valueReg);
	//mips_code << "valueReg:" + valueReg << endl;
	if (valueReg != "$t1") {
		mips_code << "    move " + valueReg + " $t1" << endl;
	}
}

void storeArrSentence(middleCode midCode) {
	mips_code << "# store_arr " + midCode.op1 + " -> " + midCode.op2 + "[" + midCode.op3 + "]" << endl;
	string valueReg = "$t0";
	string warn = loadVarToReg(midCode.op1, "0", valueReg);
	string regS = valueReg;
	storeRegToVar(midCode.op2, midCode.op3, valueReg);
	assert(valueReg == regS);
}

//func's clash happened
void callFuncSentence(middleCode midCode) {
	mips_code << "# call function " + midCode.op1 << endl;

	//cal stack offset
	int offset = 0;
	for (unsigned int i = stackIndex; i < stackTable.size(); i++) {
		offset += stoi(stackTable[i].op2) + 1;
	}
	offset = offset << 2;
	mips_code << "    addi $s0 $sp " + to_string(-offset) << endl;

	int i, num;
	for (i = 0; i < paraList.size(); i++) {
		mips_code << "#       para:                     " + paraList[i].op2 << endl;
	}
	if (funcParaRecord[midCode.op1].size() == 0) {
		num = 0;
		i = paraList.size();
	}
	else {
		for (i = paraList.size() - 1; i >= 0; i--)
			if (paraList[i].op2 == "1")
				break;
		num = paraList.size() - i;
	}
	//cout << "#### " + to_string(num)+"  "+to_string(i)+ "  "+to_string(paraList.size())<< endl;
	for (; i < (int)paraList.size(); i++) {
		string reg = "$t1";
		string warn = loadVarToReg(paraList[i].op1, "0", reg);
		mips_code << "# para_name:" + paraList[i].op1 << endl;
		int index = -((stoi(paraList[i].op2) - 1) * 4 + 8);
		mips_code << "    sw " + reg + " " + to_string(index) + "($s0)" << endl;
	}
	//paraStack pop_back
	while (num--) {
		paraList.pop_back();
	}
	globalManager.jalFunc(funcNow, midCode.op1);
	string reg = "$t1";
	mips_code << "    move $sp $s0" << endl;
	transLi(to_string(offset), reg);
	//mips_code << "    li $t1 " + to_string(offset) << endl;
	mips_code << "    sw " + reg << " 0($s0)" << endl;
	mips_code << "    jal " + midCode.op1 << endl;
	//mips_code << "    sw  $ra -4($sp)" << endl;
	globalManager.raFunc(funcNow, midCode.op1);
}

void pushParaSentence(middleCode midCode) {
	int remain, all;
	all = stoi(midCode.op3) >> 16;
	remain = stoi(midCode.op3) << 16 >> 16;
	mips_code << "# all:" + to_string(all) + " index:" + to_string(all - remain) << endl;
	midCode.op2 = to_string(all - remain);
	midCode.op3 = to_string(all);
	paraList.push_back(midCode);
}

string conditionReg;

void conditionSentence(middleCode midCode) {
	mips_code << "# " + midCode.op2 + " " + midCode.op1 + " " + midCode.op3 << endl;
	conditionReg = "$t0";
	string reg1 = "$t0", reg2 = "$t1"; 
	string warn;
	bool isConst2, isConst3;
	int val2, val3;
	isConst2 = isNumStr(midCode.op2, val2);
	isConst3 = isNumStr(midCode.op3, val3);
	if (isConst2 && isConst3) {
		warn = loadVarToReg(to_string(val2 - val3), "0", conditionReg);
	}
	else if (!isConst2 && isConst3) {
		warn = loadVarToReg(midCode.op2, "0", reg1);
		mips_code << "    addi " + conditionReg + " " + reg1 + " " + to_string(-val3) << endl;
	}
	else if (isConst2 && !isConst3) {
		//exit(-1);
	}
	else {
		warn = loadVarToReg(midCode.op2, "0", reg1);
		warn = loadVarToReg(midCode.op3, "0", reg2);
		mips_code << "    sub " + conditionReg + " " + reg1 + " " + reg2 << endl;
	}
}

void conditionJumpSentence(middleCode midCode) {
	if (condi(">=", "BNZ") || condi("<", "BZ")) {
		mips_code << "    bgez " + conditionReg + " "  + midCode.op1 << endl;
	}
	else if (condi(">", "BNZ") || condi("<=", "BZ")) {
		mips_code << "    bgtz " + conditionReg + " " + midCode.op1 << endl;
	}
	else if (condi("<=", "BNZ") || condi(">", "BZ")) {
		mips_code << "    blez " + conditionReg + " " + midCode.op1 << endl;
	}
	else if (condi("<", "BNZ") || condi(">=", "BZ")) {
		mips_code << "    bltz " + conditionReg + " " + midCode.op1 << endl;
	}
	else if (condi("==", "BNZ") || condi("!=", "BZ")) {
		mips_code << "    beq " + conditionReg + " $0 " + midCode.op1 << endl;
	}
	else if (condi("!=", "BNZ") || condi("==", "BZ")) {
		mips_code << "    bne " + conditionReg + " $0 " + midCode.op1 << endl;
	}
}

void constDefineSentence(middleCode midCode) {
	//for local const
	mips_code << "# " +  midCode.op1 + " -> " + midCode.op3 << endl;
	mips_code << "    li $t0 " + midCode.op3 << endl;
	mips_code << "# size:" + to_string(stackTable.size()) + " index:" + to_string(stackIndex) << endl;
	mips_code << "    sw $t0 -" + to_string((stackTable.size() - stackIndex) * 4) + "($sp)" << endl;
	dispatch(midCode.op1, midCode.op2, "0");
	//stackTable.push_back(middleCode(midCode.op2, midCode.op1, "0", ""));
}

void varDefineSentence(middleCode midCode) {
	//for local var
	dispatch(midCode.op1, midCode.op2, midCode.op3);
	//stackTable.push_back(middleCode(midCode.op2, midCode.op1, midCode.op3, ""));
}


void sentence(middleCode midCode,int index) {
	printMiddleCode(midCode);
	int no = getIrBlockNo(index);
	LocalManager.updateBlock(no);
	globalManager.setIntensity(midCode.intensity);
	if (midCode.Type == "MAINHEAD") {
		funcNow = "MAIN";
	}
	else if (midCode.Type == "SCAN") {
		scanSentence(midCode);
	}
	else if (midCode.Type == "PRINT") {
		printSentence(midCode);
	}
	else if (midCode.Type == "ADD" || midCode.Type == "SUB" || midCode.Type == "MUL" || midCode.Type == "DIV") {
		calSentence(midCode);
	}
	else if (midCode.Type == "MOV") {
		mips_code << "# move " + midCode.op1 + " " + midCode.op2 << endl;
		mips_code << "    move " + midCode.op1 + " " + midCode.op2 << endl;
	}
	else if (midCode.Type == "LOAD_ARR") {
		loadArrSentence(midCode);
	}
	else if (midCode.Type == "STORE_ARR") {
		storeArrSentence(midCode);
	}
	else if (midCode.Type == "CALL_FUNC") {
		callFuncSentence(midCode);
	}
	else if (midCode.Type == "PUSH_PARA") {
		pushParaSentence(midCode);
	}
	else if (midCode.Type == "LABEL") {
		mips_code << midCode.op1 + ":" << endl;
	}
	else if (midCode.Type == "GOTO") {
		mips_code << "    j " + midCode.op1 << endl;
	}
	else if (midCode.Type == "CONDITION") {
		conditionSentence(midCode);
	}
	else if (midCode.Type == "BNZ" || midCode.Type == "BZ") {
		conditionJumpSentence(midCode);
	}
	else if (midCode.Type == "CONST") {
		if (!constSpread)
			constDefineSentence(midCode);
	}
	else if (midCode.Type == "VAR") {
		varDefineSentence(midCode);
	}
	else if (midCode.Type == "EXIT") {
		mips_code << "########################### exit:" << endl;
		mips_code << "######## program exit" << endl;
		mips_code << "    li $v0 10" << endl;
		mips_code << "    syscall" << endl;
	}
	else if (midCode.Type == "FUNC") {


		stackIndex = 0;//reset stack point
		stackTable.clear();
		/* dispatch space for func var */
		dispatch("_offset", "INT", "0");
		dispatch("_ra", "INT", "0");
		vector<middleCode> nameList = funcParaRecord[midCode.op1];
		for (unsigned int i = 0; i < nameList.size(); i++) {
			dispatch(nameList[i].op1, nameList[i].Type, "0");
		}
		//store ra
		mips_code << midCode.op1 + ":" << endl;
		mips_code << "    sw $ra -4($sp)" << endl;
		mips_code << "# in func______" << endl;
		funcNow = midCode.op1;
	}
	else if (midCode.Type == "FUNCHEAD") {
		//
	}
	else if (midCode.Type == "FUNCRET") {
	if (midCode.op1 != "") {
		string reg = "$v0";
		string warn = loadVarToReg(midCode.op1, "0", reg);
		if (reg == "$0") {
			mips_code << "    add $v0 $0 $0" << endl;
		}
		else if (reg != "$v0") {
			mips_code << "    move $v0 " + reg << endl;
		}
	}
		mips_code << "    lw $t1 0($sp)" << endl;
		mips_code << "    lw $ra -4($sp)" << endl;
		mips_code << "    add $sp $sp $t1" << endl;
		mips_code << "    jr $ra" << endl;
	}
	else if (midCode.Type == "FUNCTAIL") {
	//recover ra and sp 
		mips_code << "# out func______" << endl;
	}
	else if (midCode.Type == "PARA") {
		set<string> crossSet = crossBlockTable[funcNow];
		loadStoreType = "memory";
		string regs = globalManager.getVarReg(midCode.op1, funcNow);
		loadStoreType = "";

		if (regs != "NOFOUND") {
			loadStoreType = "memory";
			string reg = globalManager.getVarReg(midCode.op1, funcNow);
			string warn = loadVarToReg(midCode.op1, "0", reg);
			loadStoreType = "";
		}

		else if (crossSet.find(midCode.op1) == crossSet.end()) {
			string reg = "$t0";
			string warn = loadVarToReg(midCode.op1, "0", reg);
			assert(reg != "$t0");
			loadStoreType = "memory";
			warn = loadVarToReg(midCode.op1, "0", reg);
			loadStoreType = "";
		}
	}
}

void dotText_gen() {
	mips_code << ".text" << endl;
	bool isGlobal = true;
	int funcbeginIndex = -1, mainbeginIndex = -1;
	// handle main 
	LocalManager.reset();
	funcNow = "MAIN";
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		if (!isGlobal)
			sentence(middleTable.table[i], i);
		else if (middleTable.table[i].Type == "FUNC" && funcbeginIndex == -1) {
			funcbeginIndex = i;
		}
		else if (middleTable.table[i].Type == "MAINHEAD") {
			mainbeginIndex = i;
			isGlobal = false;
		}
	}
	// handle func
	LocalManager.reset();
	if (funcbeginIndex != -1) {
		for (int i = funcbeginIndex; i < mainbeginIndex; i++) {
			sentence(middleTable.table[i], i);
		}
	}
}

void diffFuncAndExit() {
	int isMain = false;
	vector<middleCode> mid = middleTable.table;
	ofstream midNowFile;
	midNowFile.open("middle.txt");
	for (unsigned int i = 0; i < mid.size(); i++) {
		if (mid[i].Type == "MAINHEAD") {
			isMain = true;
		}
		if (isMain && mid[i].Type == "FUNCRET") {
			middleTable.table[i].Type = "EXIT";
		}
		midNowFile << "    " << middleTable.table[i].Type << "    " << middleTable.table[i].op1 << "    " << middleTable.table[i].op2 << "    " << middleTable.table[i].op3 << endl;
	}
	midNowFile.close();
}

void printStackTable() {
	mips_code << "#######################################################" << endl;
	for (unsigned int i = 0; i < stackTable.size(); i++) {
		mips_code << "#" + to_string(i) + ":" + stackTable[i].op1 << endl;
	}
}

void printMiddleCode(middleCode mid) {
	mips_code << "#midCode " + mid.Type + " " + mid.op1 + " " + mid.op2 + " " + mid.op3 << endl;
	cout << "#midCode " + mid.Type + " " + mid.op1 + " " + mid.op2 + " " + mid.op3 << endl;
}

void transLi(string value, string& regName) {
	if (value == "0") {
		regName = "$0";
		return;
	}
	mips_code << "    li " + regName + " " + value << endl;
}

bool cmp(const pair<string, int>& a, const pair<string, int>& b) {
	return a.second > b.second;
}

void globalRegManager::init(map<string, map<string, int>> countMap) {
	cout << "construct globalManager begin" << endl;
	

	for (map<string, map<string, int>>::iterator it = countMap.begin(); it != countMap.end(); it++) {
		regPool.clear();
		for (int i = lowRegs; i <= highRegs; i++) {
			regPool.push_back("$s" + to_string(i));
		}
		for (int i = lowRega; i <= highRega; i++) {
			regPool.push_back("$a" + to_string(i));
		}
		string funcName = it->first;
		
		cout << "##################################\n";
	    cout << funcName << endl;
		vector<pair<string, int>> useCount;
		for (map<string, int>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++) {
			useCount.push_back(*it1);
			cout << it1->first << endl;
		}
		sort(useCount.begin(), useCount.end(), cmp);
		map<string, string> name2Reg;
		map<string, string> reg2Name;
		map<string, int> name2Remain;
		set<string> crossSet = crossBlockTable[funcName];
		printSet(crossSet);
		for (vector<pair<string, int>>::iterator it1 = useCount.begin(); it1 != useCount.end(); it1++) {
			if (it1->second >= intensityBound && !regPool.empty() && crossSet.find(it1->first) != crossSet.end()) {//·ÀÖ¹ÎÊÌâ
				name2Reg.insert(make_pair(it1->first, regPool[0]));
				reg2Name.insert(make_pair(regPool[0], it1->first));
				name2Remain.insert(*it1);
				cout << regPool[0] << "->" << it1->first << endl;
				usingRegPool.insert(regPool[0]);
				regPool.erase(regPool.begin());
				//cout << "remainReg:" + to_string(regPool.size()) << endl;
			}
		}
		fun2Name2Reg.insert(make_pair(funcName, name2Reg));
		fun2Reg2Name.insert(make_pair(funcName, reg2Name));
		fun2Name2Remain.insert(make_pair(funcName, name2Remain));
	}
	cout << "construct globalManager end" << endl;
}

bool globalRegManager::isGlobalVar(string name, string funcNow) {
	set<string> crossSet = crossBlockTable[funcNow];
	//cout << to_string(crossSet.find(name) != crossSet.end()) << endl;
	//cout << to_string(fun2Name2Reg[funcNow].find(name) != fun2Name2Reg[funcNow].end()) << endl;
	if (crossSet.find(name) != crossSet.end()) {
		if (fun2Name2Reg[funcNow].find(name) != fun2Name2Reg[funcNow].end()) {
			return true;
		}
	}
	return false;
}

void globalRegManager::setIntensity(int intensity) {
	intensityNow = intensity;
}

int globalRegManager::useGlobalVarMark(string name, string funcNow) {
	string reg = fun2Name2Reg[funcNow][name];
	fun2Name2Remain[funcNow][name] -= intensityNow;
	cout << name + ":" + to_string(fun2Name2Remain[funcNow][name])<<endl;
	if (fun2Name2Remain[funcNow][name] == 0) {
		fun2Name2Reg[funcNow].erase(fun2Name2Reg[funcNow].find(name));
		fun2Reg2Name[funcNow].erase(fun2Reg2Name[funcNow].find(reg));
		cout << "disconnect " + name + " with " + reg << endl;
	}
	if (regex_match(reg, regex("\\$s[0-9]"))) {
		//cout << "index:"+ to_string(regs(stoi(reg.substr(2)))) << endl;
		//cout << "regNo:" + to_string(regs(regs(stoi(reg.substr(2))))) << endl;
		return regs(stoi(reg.substr(2)));

	}
	if (regex_match(reg, regex("\\$a[0-9]")))
		return rega(stoi(reg.substr(2)));
	exit(-1);
}


void globalRegManager::jalFunc(string funcNow, string jalFunc) {
	vector<string> clashNameList;
	if (recursiveFuncSet.find(jalFunc) == recursiveFuncSet.end()) {
		cout << "push stack local" << endl;
		for (map<string, string>::iterator it = fun2Reg2Name[jalFunc].begin(); it != fun2Reg2Name[jalFunc].end(); it++) {
			string regName = it->first;
			if (fun2Reg2Name[funcNow].find(regName) != fun2Reg2Name[funcNow].end()) {
				string name = fun2Reg2Name[funcNow].find(regName)->second;
				clashNameList.push_back(name);
				loadStoreType = "memory";
				cout << "push " + name << endl;
				storeRegToVar(name, "0", regName);
				loadStoreType = "";
			}
		}
	}
	else {
		cout << "push stack global" << endl;
		for (map<string, string>::iterator it = fun2Reg2Name[funcNow].begin(); it != fun2Reg2Name[funcNow].end(); it++) {
			string regName = it->first;
			string name = fun2Reg2Name[funcNow].find(regName)->second;
			clashNameList.push_back(name);
			loadStoreType = "memory";
			cout << "push " + name << endl;
			storeRegToVar(name, "0", regName);
			loadStoreType = "";
		}
	}
	clashNameNow = clashNameList;
}

void globalRegManager::raFunc(string funcNow, string jalFunc) {
	for (size_t i = 0; i < clashNameNow.size(); i++) {
		string name = clashNameNow[i];
		string reg = fun2Name2Reg[funcNow][name];
		loadStoreType = "memory";
		string warn = loadVarToReg(name, "0", reg);
		loadStoreType = "";
	}
	clashNameNow.clear();
}

string globalRegManager::getVarReg(string name, string funcNow) {
	string type, globalName;
	if (loadStoreType != "")
		searchData(name, globalName, type);
	if (fun2Name2Reg[funcNow].find(name) != fun2Name2Reg[funcNow].end()) {
		return fun2Name2Reg[funcNow][name];
	}
	return "NOFOUND";
}

void translate() {
	//handle the situation of return of main
	globalManager.init(impVarCountMap);
	mips_code.open("mips.txt");
	dotData_gen();//generate .data
	dotText_gen();//generate .text
	printStackTable();
}


