#include "includeH.h"

vector<BB> block;
vector<int> endBlockArr;
//global defined by user
set<string> globalSet;
//lively through the block
map<string, set<string>> crossBlockTable;
set<string> arraySet;

map<string, map<string, int>> impVarCountMap;
set<string> recursiveFuncSet;

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
		if (type == "MAINHEAD" || type == "FUNC" || type == "FUNCRET" || type == "GOTO" || type == "BNZ" || type == "BZ" || type == "EXIT" || type == "CALL_FUNC") {
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
			if (middleTable.table[j].Type == "FUNC") {
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

void definedConstSpread() {
	cout << "defined Const Spread begin" << endl;
	vector<middleCode> table = middleTable.table;
	vector<middleCode> constTable;
	for (size_t i = 0; i < table.size(); i++) {
		table[i].no = i;
		if (table[i].Type == "CONST") {
			constTable.push_back(table[i]);
		}
		bool ops1 = false, ops2 = false, ops3 = false;
		if (table[i].Type == "ADD" || table[i].Type == "SUB" || table[i].Type == "MUL" || table[i].Type == "DIV") {
			ops2 = true;
			ops3 = true;
		}
		if (table[i].Type == "LOAD_ARR" || table[i].Type == "STORE_ARR") {
			ops3 = true;
			if (table[i].Type == "STORE_ARR") {
				ops1 = true;
			}
		}
		if (table[i].Type == "PUSH_PARA" || table[i].Type == "SCAN") {//avoid using printf prevent str's influence 
			ops1 = true;
		}
		if (table[i].Type == "PRINT" && table[i].op2 == "1") {
			if (isVarStr(table[i].op1)) {
				ops1 = true;
			}
		}
		if (table[i].Type == "CONDITION") {
			ops2 = true;
			ops3 = true;
		}
		if (table[i].Type == "FUNCRET") {
			ops1 = true;
		}
		//cout << to_string(i) +":"+ to_string(op1) + to_string(op2) + to_string(op3) << endl;
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
	}

	cout << "defined Const Spread finished" << endl;
	cout << "##################################################################" << endl;
}

//analyse var type(local or global) and const spread
void varTypeAnalyse() {
	cout << "cross var analyse begin" << endl;
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
		//cout << to_string(i) +":"+ to_string(op1) + to_string(op2) + to_string(op3) << endl;
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

	map<string, set<string>> definedTable;
	string funcName = "";
	set<string> definedSet;
	cout << "definedTable!!" << endl;
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		if (middleTable.table[i].Type == "FUNC") {
			if (funcName != "") {
				definedTable.insert(make_pair(funcName, definedSet));
				printSet(definedSet);
				definedSet.clear();
			}
			funcName = middleTable.table[i].op1;
		}
		else if (middleTable.table[i].Type == "MAINHEAD") {
			if (funcName != "") {
				definedTable.insert(make_pair(funcName, definedSet));
				printSet(definedSet);
				definedSet.clear();
			}
			funcName = "MAIN";
		}
		else if (middleTable.table[i].Type == "EXIT") {
			definedTable.insert(make_pair(funcName, definedSet));
			printSet(definedSet);

		}
		if (funcName != "") {
			if (middleTable.table[i].Type == "VAR") {
				definedSet.insert(middleTable.table[i].op1);
			} 
			else if (middleTable.table[i].Type == "CONST") {
				definedSet.insert(middleTable.table[i].op1);
			}
			else if (middleTable.table[i].Type == "PARA") {
				definedSet.insert(middleTable.table[i].op1);
			}
		}
	}

	cout << "inOutTable!!" << endl;
	map<string, set<string>> inOutTable;
	set<string> inOutSet;
	funcName = "";
	for (vector<BB>::iterator it = block.begin(); it != block.end(); it++) {
		string funcNow = it->funcName;
		cout << funcName + "->" + funcNow << endl;
		if (funcName == "" || funcName == "GLOBAL") {
			funcName = funcNow;
		}
		else if (funcNow != funcName){
			inOutTable.insert(make_pair(funcName, inOutSet));
			printSet(inOutSet);
			inOutSet.clear();
			funcName = funcNow;
		}
		for (set<string>::iterator it1 = it->lvIn.begin(); it1 != it->lvIn.end(); it1++) {
			inOutSet.insert(*it1);
		}
		for (set<string>::iterator it1 = it->lvOut.begin(); it1 != it->lvOut.end(); it1++) {
			inOutSet.insert(*it1);
		}
	}
	inOutTable.insert(make_pair(funcName, inOutSet));
	printSet(inOutSet);

	for (map<string, set<string>>::iterator it = definedTable.begin(); it != definedTable.end(); it++) {
		string funcName = it->first;
		if (funcName == "GLOBAL") {
			continue;
		}
		set<string> definedSet = definedTable[funcName];
		set<string> inOutSet = inOutTable[funcName];
		set<string> crossSet;
		for (set<string>::iterator it1 = definedSet.begin(); it1 != definedSet.end(); it1++) {
			if (inOutSet.find(*it1) != inOutSet.end()) {
				crossSet.insert(*it1);
			}
		}
		for (set<string>::iterator it1 = inOutSet.begin(); it1 != inOutSet.end(); it1++) {
			if (isTemp(*it1)) {
				crossSet.insert(*it1);
			}
		}
		cout << funcName << endl;
		printSet(crossSet);
		crossBlockTable.insert(make_pair(funcName, crossSet));
	}
	cout << "cross var analyse finished" << endl;
	cout << "##################################################################" << endl;
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
			else if (mid.Type == "PARA") {
				set<string>::iterator it1 = useSet.find(mid.op1);
				varStatics(varCount, mid.op1);
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
					it->Type = "ADD";
					it->op2 = to_string(val2 - val3);
					it->op3 = "0";
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
		else if (it->Type == "CONDITION") {
			bool isConst2, isConst3;
			int val2, val3;
			isConst2 = isNumStr(it->op2, val2);
			isConst3 = isNumStr(it->op3, val3);
			if (isConst2 && isConst3 && val3 != 0) {
				isOpt = true;
				it->op2 = to_string(val2 - val3);
				it->op3 = "0";
			}
			else if (isConst2 && !isConst3) {//SUB CONST VAR
				isOpt = true;
				if (it->op1 == "BNZ") {
					it->op1 = "BZ";
				}
				else {
					it->op1 = "BNZ";
				}
				string temp = it->op2;
				it->op2 = it->op3;
				it->op3 = temp;
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

void change_intensity(middleCode mid, int& intensity) {
	if (mid.Type == "LABEL") {
		if (regex_match(mid.op1, regex("for_Begin\\d+"))) {
			intensity *= 5;
		}
		else if (regex_match(mid.op1, regex("while_Begin\\d+"))) {
			intensity *= 5;
		}
		else if (regex_match(mid.op1, regex("do_while_Begin\\d+"))) {
			intensity *= 5;
		}
		else if (regex_match(mid.op1, regex("while_End\\d+"))) {
			intensity /= 5;
		}
		else if (regex_match(mid.op1, regex("for_End\\d+"))) {
			intensity /= 5;
		}
		
	}
	if (mid.Type == "BZ" || mid.Type == "BNZ") {
		if (regex_match(mid.op1, regex("do_while_Begin\\d+"))) {
			intensity /= 5;
		}
	}
}

//analyse var and dispatch func global reg
void ImpVarCount() {
	string funcBlock = "GLOBAL";
	int useIntensity = 1;
	map<string, int> impVarCount;
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		string funcNow = block[getIrBlockNo(i)].funcName;
		//change intensity
		if (funcNow != funcBlock) {
			if (!impVarCount.empty()) {
				cout << "!!!!!!!!!!!!!!!!!!!" + funcBlock + "???????????????????" << endl;
				impVarCountMap.insert(make_pair(funcBlock, impVarCount));
				printMap(impVarCount);
			}
			funcBlock = funcNow;
			assert(useIntensity == 1);
			impVarCount.clear();
		}
		else {
			change_intensity(middleTable.table[i],useIntensity);
		}
		middleTable.table[i].intensity = useIntensity;
		//analyse the func call
		if (middleTable.table[i].Type == "CALL_FUNC") {
			recursiveFuncSet.insert(funcNow);
		}
		//cout << "#midCode " + middleTable.table[i].Type + " " + middleTable.table[i].op1 + " " + middleTable.table[i].op2 + " " + middleTable.table[i].op3 << endl;

		//cout << to_string(useIntensity) << endl;

		
		bool op1 = false, op2 = false, op3 = false;
		if (middleTable.table[i].Type == "ADD" || middleTable.table[i].Type == "SUB" || middleTable.table[i].Type == "MUL" || middleTable.table[i].Type == "DIV") {
			op1 = true;
			op2 = true;
			op3 = true;
		}
		if (middleTable.table[i].Type == "LOAD_ARR" || middleTable.table[i].Type == "STORE_ARR") {
			op1 = true;
			op3 = true;
		}
		if (middleTable.table[i].Type == "PUSH_PARA" || middleTable.table[i].Type == "SCAN") {//avoid using printf prevent str's influence 
			op1 = true;
		}
		if (middleTable.table[i].Type == "PRINT" && middleTable.table[i].op2 == "1") {
			if (isVarStr(middleTable.table[i].op1)) {
				op1 = true;
			}
		}
		if (middleTable.table[i].Type == "CONDITION") {
			op2 = true;
			op3 = true;
		}
		if (middleTable.table[i].Type == "FUNCRET") {
			op1 = true;
		}
		string str1, str2, str3;
		str1 = middleTable.table[i].op1;
		str2 = middleTable.table[i].op2;
		str3 = middleTable.table[i].op3;
		set<string> crossSet = crossBlockTable[funcBlock];
		if (op1 && crossSet.find(str1) != crossSet.end()) {
			if (impVarCount.find(str1) == impVarCount.end()) {
				impVarCount.insert(make_pair(str1, useIntensity));
			}
			else {
				impVarCount[str1] = impVarCount[str1] + useIntensity;
			}
			cout << str1 + ":" + to_string(impVarCount[str1]) << endl;
		} 
		if (op2 && crossSet.find(str2) != crossSet.end()) {
			if (impVarCount.find(str2) == impVarCount.end()) {
				impVarCount.insert(make_pair(str2, useIntensity));
			}
			else {
				impVarCount[str2] = impVarCount[str2] + useIntensity;
			}
			cout << str2 + ":" + to_string(impVarCount[str2]) << endl;
		}
		if (op3 && crossSet.find(str3) != crossSet.end()) {
			if (impVarCount.find(str3) == impVarCount.end()) {
				impVarCount.insert(make_pair(str3, useIntensity));
			}
			else {
				impVarCount[str3] = impVarCount[str3] + useIntensity;
			}
			cout << str3 + ":" + to_string(impVarCount[str3]) << endl;
		}
	}
	impVarCountMap.insert(make_pair("MAIN", impVarCount));
	printImpVarCount();
}

void printImpVarCount() {
	for (map<string, map<string, int>>::iterator it = impVarCountMap.begin(); it != impVarCountMap.end(); it++) {
		cout << "##############impVarCountMap################"<<endl<<it->first <<":"<< endl;
		for (map<string, int>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++) {
			cout << it1->first << ":" + to_string(it1->second) << endl;
		}
	}
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
	divideBlock();
	definedConstSpread();
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

	ImpVarCount();

	ofstream opt_mid;
	opt_mid.open("opt_middle.txt");
	for (size_t i = 0; i < middleTable.table.size(); i++) {
		middleCode mid = middleTable.table[i];
		opt_mid << "    " << mid.Type << "    " << mid.op1 << "    " << mid.op2 << "    " << mid.op3 << endl;
	}
}