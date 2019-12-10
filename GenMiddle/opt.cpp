#include "includeH.h"

vector<BB> block;
vector<int> endBlockArr;
//global defined by user
set<string> globalSet;
//lively through the block
set<string> globalBlockSet;
set<string> arraySet;


BB::BB(int num, int first, int last,string func) {
	no = num;
	firstLoc = first;
	lastLoc = last;
	funcName = func;
}

int getIrBlockNo(int index) {
	for (size_t i = 0; i < block.size(); i++) {
		if (block[i].firstLoc < index && index <= block[i].lastLoc) {
			return i;
		}
	}
	if (middleTable.table[index].Type == "EXIT") {
		return block.size() - 1;
	}
	return -1;
}

int getLabelBlockNo(string label) {
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		middleCode mid = middleTable.table[i];
		if (mid.Type == "LABEL" && mid.op1 == label) {
			return getIrBlockNo(i);
		}
	}
	return -1;
}

void printBlock() {
	for (size_t i = 0; i < block.size(); i++){
		cout << "no:" + to_string(block[i].no) << endl;
		cout << "  func:" + block[i].funcName << endl;
		cout << "  first:" + to_string(block[i].firstLoc) << endl;
		cout << "  last:" + to_string(block[i].lastLoc) << endl;
		cout << "    prevBB:";
		for (size_t j = 0; j < block[i].prevBB.size(); j++) {
			cout << to_string(block[i].prevBB[j]) << " ";
		}
		cout << endl;

		cout << "    nextBB:";
		for (size_t j = 0; j < block[i].nextBB.size(); j++) {
			cout << to_string(block[i].nextBB[j]) << " ";
		}
		cout << endl;

		cout << "    varCount:" << endl;
		for (map<string, int>::iterator it = block[i].varCount.begin(); it != block[i].varCount.end(); it++) {
			cout << "####" + it->first + ":" + to_string(it->second) << endl;
		}
		cout << endl;

		cout << "      used:";
		for (set<string>::iterator it = block[i].usedSet.begin(); it != block[i].usedSet.end(); it++) {
			cout << *it << " ";
		}
		cout << endl;
		cout << "      use:";
		for (set<string>::iterator it = block[i].useSet.begin(); it != block[i].useSet.end(); it++) {
			cout << *it << " ";
		}

		cout << endl;
		cout << "      def:";
		for (set<string>::iterator it = block[i].defSet.begin(); it != block[i].defSet.end(); it++) {
			cout << *it << " ";
		}
		cout << endl;

		cout << "      lvIn:";
		for (set<string>::iterator it = block[i].lvIn.begin(); it != block[i].lvIn.end(); it++) {
			cout << *it << " ";
		}
		cout << endl;

		cout << "      lvOut:";
		for (set<string>::iterator it = block[i].lvOut.begin(); it != block[i].lvOut.end(); it++) {
			cout << *it << " ";
		}
		cout << endl;
		cout << "        middleCode" << endl;
		for (int j = block[i].firstLoc + 1; j <= block[i].lastLoc; j++) {
			middleCode mid = middleTable.table[j];
			cout << "#midCode " + mid.Type + " " + mid.op1 + " " + mid.op2 + " " + mid.op3 << endl;
		}
	}
}

void divideBlock() {
	vector<int> divideIndex;
	/*for (size_t i = 0; i < middleTable.table.size(); i++) {
		middleCode mid = middleTable.table[i];
		cout << "    " << mid.Type << "    " << mid.op1 << "    " << mid.op2 << "    " << mid.op3 << endl;

	}*/
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		string type = middleTable.table[i].Type;
		if (type == "MAINHEAD" || type == "FUNCHEAD" || type == "FUNCRET" || type == "GOTO" || type == "BNZ" || type == "BZ" || type == "EXIT" || type == "CALL_FUNC") {
			//cout << to_string(i) +":"+type<< endl;
			if (divideIndex.empty()) {
				divideIndex.push_back(i);
			}
			if (!divideIndex.empty() && divideIndex[divideIndex.size() - 1] != i) {
				divideIndex.push_back(i);
			}
		}
		else if (type == "LABEL") {
			//cout << to_string(i) + ":" + type << endl;
			if (divideIndex.empty()) {
				divideIndex.push_back(i-1);
			}
			if (!divideIndex.empty() && divideIndex[divideIndex.size() - 1] != i - 1) {
				divideIndex.push_back(i-1);
			}
		}
	}

	block.clear();
	endBlockArr.clear();
	//cout << "q" << endl;
	block.push_back(BB(0, -1, divideIndex[0], "GLOBAL"));
	for (size_t i = 0; i < divideIndex.size() - 1; i++) {
		string funcName = "";
		for (size_t j = 0; j < middleTable.table.size(); j++) {
			if (middleTable.table[j].Type == "FUNCHEAD") {
				funcName = middleTable.table[j].op1;
			}
			if (middleTable.table[j].Type == "MAINHEAD") {
				funcName = "MAIN";
			}
			if (int(j) >= divideIndex[i])
				break;
		}
		cout << to_string(divideIndex[i]) << " -> " + to_string(divideIndex[i + 1]) << endl;
		block.push_back(BB(i+1, divideIndex[i], divideIndex[i + 1],funcName));
	}

	for (size_t i = 0; i < middleTable.table.size(); i++) {
		string type = middleTable.table[i].Type;
		if (type == "GOTO" || type == "BNZ" || type == "BZ") {
			int no1 = getIrBlockNo(i);
			int no2 = getLabelBlockNo(middleTable.table[i].op1);
			block[no1].nextBB.push_back(no2);
			block[no2].prevBB.push_back(no1);
		}
		else if (type == "FUNCRET" || type == "EXIT") {
			int no1 = getIrBlockNo(i);
			endBlockArr.push_back(no1);
		}
	}

	for (vector<BB>::iterator it = block.begin(); it != block.end() - 1; it++) {
		string type = middleTable.table[it->lastLoc].Type;
		if (type != "GOTO" && type != "FUNCRET" && type != "EXIT") {
			int no = it->no;
			block[no].nextBB.push_back(no + 1);
			block[no + 1].prevBB.push_back(no);
		}
	}

	/*for (vector<BB>::iterator it = block.begin(); it != block.end() - 1; it++) {
		if (it->nextBB.empty() && it->prevBB.empty()) {
			it = block.erase(it) - 1;
		}
	}*/
	//printBlock();
}

//analyse var type(local or global) and const spread
void varTypeAnalyse() {
	set<string> varSet;
	vector<middleCode> table = middleTable.table;
	vector<middleCode> constTable;
	for (size_t i = 0; i < table.size(); i++) {
		table[i].no = i;
		if (table[i].Type == "CONST") {
			constTable.push_back(table[i]);
		}
		bool arrOp = false;
		bool op1 = false, op2 = false, op3 = false, ops1 = false, ops2 = false, ops3 = false;
		if (table[i].Type == "ADD" || table[i].Type == "SUB" || table[i].Type == "MUL" || table[i].Type == "DIV") {
			op1 = true;
			op2 = true;
			op3 = true;
			ops2 = true;
			ops3 = true;
		}
		if (table[i].Type == "LOAD_ARR" || table[i].Type == "STORE_ARR") {
			arrOp = true;
			op1 = true;
			op3 = true;
			ops3 = true;
			if (table[i].Type == "STORE_ARR") {
				ops1 = true;
			}
		}
		if (table[i].Type == "PUSH_PARA" || table[i].Type == "SCAN") {//avoid using printf prevent str's influence 
			op1 = true;
			ops1 = true;
		}
		if (table[i].Type == "PRINT" && table[i].op2 == "1") {
			if (isVarStr(table[i].op1)) {
				op1 = true;
				ops1 = true;
			}
		}
		if (table[i].Type == "CONDITION") {
			op2 = true;
			op3 = true;
			ops2 = true;
			ops3 = true;
		}
		if (table[i].Type == "FUNCRET") {
			op1 = true;
			ops1 = true;
		}
		if (op1 && isVarStr(table[i].op1)) {
			varSet.insert(table[i].op1);
		}
		if (op2 && isVarStr(table[i].op2)) {
			varSet.insert(table[i].op2);
		}
		if (op3 && isVarStr(table[i].op3)) {
			varSet.insert(table[i].op3);
		}
		cout << to_string(i) +":"+ to_string(op1) + to_string(op2) + to_string(op3) << endl;
		string nowFuncName = block[getIrBlockNo(table[i].no)].funcName;
		if (ops1 && constSpread) {
			for (int j = constTable.size() - 1; j >= 0; j--) {
				string definedName = block[getIrBlockNo(constTable[j].no)].funcName;
				if (constTable[j].op1 == table[i].op1) {
					if (definedName == nowFuncName || definedName == "GLOBAL") {
						//cout << definedName + " " + nowFuncName << endl;
						middleTable.table[i].op1 = constTable[j].op3;
						break;
					}
				}
			}
		}
		if (ops2 && constSpread) {
			for (int j = constTable.size() - 1; j >= 0; j--) {
				string definedName = block[getIrBlockNo(constTable[j].no)].funcName;
				//cout << definedName + " " + nowFuncName << endl;
				if (constTable[j].op1 == table[i].op2) {
					if (definedName == nowFuncName || definedName == "GLOBAL") {
						middleTable.table[i].op2 = constTable[j].op3;
						break;
					}
				}
			}
		}
		if (ops3 && constSpread) {
			for (int j = constTable.size() - 1; j >= 0; j--) {
				string definedName = block[getIrBlockNo(constTable[j].no)].funcName;
				//cout << definedName + " " + nowFuncName << endl;
				if (constTable[j].op1 == table[i].op3) {
					if (definedName == nowFuncName || definedName == "GLOBAL") {
						middleTable.table[i].op3 = constTable[j].op3;
						break;
					}
				}
			}
		}
		if (arrOp) {
			arraySet.insert(table[i].op2);
		}
	}
	printBlock();
	cout << "##################################################################" << endl;
	cout << "#var:" << endl;
	for (set<string>::iterator it = varSet.begin(); it != varSet.end(); it++) {
		cout << "  " + *it << endl;
	}
	for (set<string>::iterator it = varSet.begin(); it != varSet.end(); it++) {
		/*if (!globalSet.empty()) {
			if (globalSet.find(*it) != globalSet.end()) {
				continue;
			}
		}*/
		for (vector<BB>::iterator it1 = block.begin(); it1 != block.end(); it1++) {
			set<string> in = it1->lvIn;
			set<string> out = it1->lvOut;
			if (!in.empty()) {
				if (in.find(*it) != in.end()) {
					globalBlockSet.insert(*it);
					break;
				}
			}
			if (!out.empty()) {
				if (out.find(*it) != out.end()) {
					globalBlockSet.insert(*it);
					break;
				}
			}
		}
	}
	cout << "##################################################################" << endl;
	cout << "#global:" << endl;
	for (set<string>::iterator it = globalBlockSet.begin(); it != globalBlockSet.end(); it++) {
		cout << "  " + *it << endl;
	}
}

void varStatics(map<string, int>& varCount, string name) {
	if (varCount.find(name) == varCount.end()) {
		varCount.insert(make_pair(name, 1));
	}
	else {
		varCount[name] = varCount[name] + 1;
	}
}

void calVarUseDefSet() {
	for (size_t i = 0; i < block.size(); i++) {
		set<string> useSet;
		set<string> defSet;
		set<string> usedSet;
		map<string, int> varCount;
		for (int j = block[i].firstLoc + 1; j <= block[i].lastLoc; j++) {
			middleCode mid = middleTable.table[j];
			if (mid.Type == "ADD" || mid.Type == "SUB" || mid.Type == "MUL" || mid.Type == "DIV") {
				usedSet.insert(mid.op2);
				usedSet.insert(mid.op3);
				varStatics(varCount, mid.op1);
				varStatics(varCount, mid.op2);
				varStatics(varCount, mid.op3);
				set<string>::iterator it2 = defSet.find(mid.op2);
				if (it2 == defSet.end()) {
					useSet.insert(mid.op2);
				}
				set<string>::iterator it3 = defSet.find(mid.op3);
				if (it3 == defSet.end()) {
					useSet.insert(mid.op3);
				}
				set<string>::iterator it1 = useSet.find(mid.op1);
				if (it1 == useSet.end()) {
					defSet.insert(mid.op1);
				}
			}
			else if (mid.Type == "LOAD_ARR" || mid.Type == "STORE_ARR") {
				varStatics(varCount, mid.op1);
				varStatics(varCount, mid.op3);
				usedSet.insert(mid.op3);
				set<string>::iterator it3 = defSet.find(mid.op3);
				if (it3 == defSet.end()) {
					useSet.insert(mid.op3);
				}
				if (mid.Type == "LOAD_ARR") {
					set<string>::iterator it1 = useSet.find(mid.op1);
					if (it1 == useSet.end()) {
						defSet.insert(mid.op1);
					}
				}
				if (mid.Type == "STORE_ARR") {
					usedSet.insert(mid.op1);
					set<string>::iterator it1 = defSet.find(mid.op1);
					if (it1 == defSet.end()) {
						useSet.insert(mid.op1);
					}
				}
			}
			else if (mid.Type == "PRINT" && mid.op2 == "1") {
				usedSet.insert(mid.op1);
				varStatics(varCount, mid.op1);
				set<string>::iterator it1 = defSet.find(mid.op1);
				if (it1 == defSet.end()) {
					useSet.insert(mid.op1);
				}
			}
			else if (mid.Type == "FUNCRET" || mid.Type == "PUSH_PARA") {
				usedSet.insert(mid.op1);
				varStatics(varCount, mid.op1);
				set<string>::iterator it1 = defSet.find(mid.op1);
				if (it1 == defSet.end()) {
					useSet.insert(mid.op1);
				}
			}
			else if (mid.Type == "SCAN") {
				varStatics(varCount, mid.op1);
				set<string>::iterator it1 = useSet.find(mid.op1);
				if (it1 == useSet.end()) {
					defSet.insert(mid.op1);
				}
			}
			else if (mid.Type == "CONDITION") {
				varStatics(varCount, mid.op2);
				varStatics(varCount, mid.op3);
				usedSet.insert(mid.op2);
				usedSet.insert(mid.op3);
				set<string>::iterator it2 = defSet.find(mid.op2);
				if (it2 == defSet.end()) {
					useSet.insert(mid.op2);
				}
				set<string>::iterator it3 = defSet.find(mid.op3);
				if (it3 == defSet.end()) {
					useSet.insert(mid.op3);
				}
			}
		}
		//cout << "blockUse:";
		//delete RET
		set<string>::iterator it = useSet.find("RET");
		if (it != useSet.end()) {
			useSet.erase(it);
		}
		map<string, int>::iterator it1 = varCount.find("RET");
		if (it1 != varCount.end()) {
			varCount.erase(it1);
		}
		//delete const and char
		for (set<string>::iterator it = useSet.begin(); it != useSet.end(); ) {
			//cout << *it+" ";
			if (!isVarStr(*it)) {
				it = useSet.erase(it);
			}
			else {
				it++;
			}
		}
		//cout << endl;
		/*cout << "blockDef:";
		for (set<string>::iterator it = defSet.begin(); it != defSet.end(); it++) {
			cout << *it + " ";
		}
		cout << endl;*/
		block[i].useSet = useSet;
		block[i].defSet = defSet;
		block[i].usedSet = usedSet;
		block[i].varCount = varCount;
	}
}


void liveVarLoop() {
	vector<int> handleList;
	set<int> handleSet;
	for (size_t i = 0; i < endBlockArr.size(); i++) {
		handleList.push_back(endBlockArr[i]);
		handleSet.insert(endBlockArr[i]);
	}
	//printBlock();
	for (size_t i = 0; i < handleList.size(); i++) {
		int no = handleList[i];
		//cout << to_string(no)<<endl;
		//BB b = block[no];
		if (!block[no].lvOut.empty()) {
			for (set<string>::iterator it = block[no].lvOut.begin(); it != block[no].lvOut.end(); it++) {
				if (block[no].defSet.find(*it) == block[no].defSet.end()) {
					block[no].lvIn.insert(*it);
				}
			}
		}
		if (!block[no].useSet.empty()) {
			for (set<string>::iterator it = block[no].useSet.begin(); it != block[no].useSet.end(); it++) {
				block[no].lvIn.insert(*it);
			}
		}
		if (!block[no].prevBB.empty()) {
			for (vector<int>::iterator it = block[no].prevBB.begin(); it != block[no].prevBB.end(); it++) {
				for (set<string>::iterator it1 = block[no].lvIn.begin(); it1 != block[no].lvIn.end(); it1++) {
					block[*it].lvOut.insert(*it1);
				}
				if (handleSet.find(*it) == handleSet.end()) {
					handleList.push_back(*it);
					handleSet.insert(*it);
				}
			}
		}
	}
}

void deadVarHandle(bool& isOpt) {
	liveVarAnalyse();
	int index = 0;
	for (vector<middleCode>::iterator it = middleTable.table.begin(); it != middleTable.table.end(); it++, index++) {
		if (it->Type == "ADD" || it->Type == "SUB" || it->Type == "MUL" || it->Type == "DIV") {
			int no = getIrBlockNo(index);
			BB b = block[no];
			if (b.lvIn.find(it->op1) == b.lvIn.end() && b.lvOut.find(it->op1) == b.lvOut.end() && b.usedSet.find(it->op1) == b.usedSet.end() && globalSet.find(it->op1) == globalSet.end()) {
				cout << to_string(no) << endl;
				it = middleTable.table.erase(it);
				isOpt = true;
				return;
			}
		}
	}
}

void liveVarAnalyse() {
	divideBlock();
	calVarUseDefSet();
	liveVarLoop();
	liveVarLoop();
	liveVarLoop();
	//printBlock();
}

void simplifyExp(bool& isOpt) {//combine and simplify
	for (vector<middleCode>::iterator it = middleTable.table.begin(); it != middleTable.table.end(); it++) {
		if (it->Type == "ADD" || it->Type == "SUB" || it->Type == "MUL" || it->Type == "DIV") {
			bool isConst2, isConst3;
			int val2, val3;
			isConst2 = isNumStr(it->op2, val2);
			isConst3 = isNumStr(it->op3, val3);
			if (it->Type == "ADD") {
				if (isConst2 && isConst3 && val3 != 0) {
					int val = val2 + val3;
					it->op2 = to_string(val);
					it->op3 = "0";
					isOpt = true;
				}
				if (isConst2 && val2 == 0 && it->op1 == it->op3) {
					it = middleTable.table.erase(it) - 1;
					isOpt = true;
				}
				if (isConst3 && val3 == 0 && it->op1 == it->op2) {
					it = middleTable.table.erase(it) - 1;
					isOpt = true;
				}
			}
			if (it->Type == "SUB") {
				if (isConst2 && isConst3) {
					it->op2 = to_string(val2 - val3);
					it->op3 = "";
					isOpt = true;
				}
				if (isConst3 && val3 == 0 && it->op1 == it->op2) {
					it = middleTable.table.erase(it) - 1;
					isOpt = true;
				}
			}
			if (it->Type == "MUL") {
				if (isConst2 && isConst3) {
					isOpt = true;
					it->Type = "ADD";
					it->op2 = to_string(val2 * val3);
					it->op3 = "0";
				}
				if (isConst2 && val2 == 1 && it->op1 == it->op3) {
					isOpt = true;
					it = middleTable.table.erase(it) - 1;
				}
				if (isConst3 && val3 == 1 && it->op1 == it->op2) {
					isOpt = true;
					it = middleTable.table.erase(it) - 1;
				}
			}
			if (it->Type == "DIV") {
				if (isConst2 && isConst3) {
					it->op1 = "ADD";
					it->op2 = to_string(val2 / val3);
					it->op3 = "0";
					isOpt = true;
				}
				if (isConst3 && val3 == 1 && it->op1 == it->op2) {
					it = middleTable.table.erase(it) - 1;
					isOpt = true;
				}
			}
		}
		

	}
}

void tempVarHandle(bool& isOpt) {
	for (vector<middleCode>::iterator it = middleTable.table.begin(); it != middleTable.table.end(); it++) {
		if (it->Type == "ADD") {
			int val;
			bool isConst = isNumStr(it->op3, val);
			bool isT = isTemp(it->op2);
			if (isT && isConst && val == 0) {
				if ((it - 1)->op1 == it->op2) {
					(it - 1)->op1 = it->op1;
					it = middleTable.table.erase(it);
					it--;
					isOpt = true;
				}
			}
		}
	}
}

void pushSentenceHandle() {
	vector<middleCode> newTable;
	vector<middleCode> pushStack;
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		middleCode mid = middleTable.table[i];
		if (middleTable.table[i].Type == "PUSH_PARA") {
			pushStack.push_back(mid);
			int remain, all;
			all = stoi(mid.op3) >> 16;
			remain = stoi(mid.op3) << 16 >> 16;
			if (remain == 0) {
				for (size_t j = 0; j < all; j++) {
					middleCode mid1 = pushStack[pushStack.size() - all + j];
					newTable.push_back(mid1);
				}
				while (all--) {
					pushStack.pop_back();
				}
			}
		}
		else {
			newTable.push_back(mid);
		}
	}
	middleTable.table = newTable;
}



void optimize() {
	cout << "opt begin" << endl;
	bool isGlobal = true;
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		if (middleTable.table[i].Type == "FUNCHEAD" || middleTable.table[i].Type == "MAINHEAD") {
			isGlobal = false;
		}
		if (isGlobal && (middleTable.table[i].Type == "VAR" || middleTable.table[i].Type == "CONST")) {
			globalSet.insert(middleTable.table[i].op1);
		}
	}

	//const spread

	pushSentenceHandle();

	bool isOpt = false;
	cout << "simplify begin" << endl;
	do {
		cout << "opting" << endl;
		isOpt = false;
		tempVarHandle(isOpt);
		//cout << to_string(isOpt) << endl;
		simplifyExp(isOpt);
		//cout << to_string(isOpt) << endl;
	} while (isOpt);
	cout << "simplify end" << endl;




	liveVarAnalyse();
	/*do {
		isOpt = false;
		deadVarHandle(isOpt);
	} while (isOpt);*/
	varTypeAnalyse();

	ofstream opt_mid;
	opt_mid.open("opt_middle.txt");
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		middleCode mid = middleTable.table[i];
		opt_mid << "    " << mid.Type << "    " << mid.op1 << "    " << mid.op2 << "    " << mid.op3 << endl;
	}
}