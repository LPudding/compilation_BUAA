#include "includeH.h"
//middle code
extern middleCodeTable middleTable;

ofstream mips_code;

vector<string> stringTable;//string array
const string stringName = "_string_";
const string varName = "_var_";
const string arrayName = "_array_";

//type name globalName length  store the global var and array
vector<middleCode> globalTable;

//type name length 
vector<middleCode> stackTable;
int stackIndex = 0;//current stack index
int lastIndex;

//type name index num
vector<middleCode> paraList;

void dotData_gen() {
	bool isGlobal = true;
	bool isMain = false;
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
		if (isMain && iter->Type == "PRINT" && iter->op2 == "0") {//str
			stringTable.push_back(iter->op1);
		}
	}
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
	for (int i = 0; i < stringTable.size(); i++) {
		mips_code << stringName + to_string(i) + " : .asciiz \"" + stringTable[i] + "\"" << endl;
		stringTable[i] = stringName + to_string(i);
	}
	mips_code << stringName + " : .asciiz \"\\n\"" << endl;
	//var and array
	for (int i = 0, j = 0, k = 0; i < globalTable.size() * 2; i++) {
		int length = stoi(globalTable[i % globalTable.size()].op3);
		if (length == 0 && i < globalTable.size()) {
			string type;
			if (globalTable[i].op2 == "") {
				type = (globalTable[i].Type == "INT") ? "word 0" : "byte 0";
			}
			else if (globalTable[i].Type == "INT"){
				type = "word " + globalTable[i].op2;
			}
			else {
				type = "byte " + globalTable[i].op2;
			}
			string name = varName + to_string(j++);
			globalTable[i].op2 = name;
			mips_code << name + " : ." + type << endl;
		}
		else if (length > 0 && i >= globalTable.size()){
			string size = to_string(stoi(globalTable[i % globalTable.size()].op3));
			string name = arrayName + to_string(k++);
			globalTable[i % globalTable.size()].op2 = name;
			mips_code <<  name + " : .word 0:" + size << endl;
		}
	}

}

int dispatchReg(string name) {
	return 0;//no free reg for now
}

/*************************************************************************************
	if data in local stack return the relative position and type    return value >= 0;
	if data in global table return the global name and type         return value = -1;
	if can't find data                                              return value = -2;
	************************************************************************************/
int searchData(string name,string& globalName,string& type) {
	int count = 0;
	for (int i = stackIndex; i < stackTable.size(); i++) {
		//mips_code << "name:" + stackTable[i].op1 << endl;
		if (stackTable[i].op1 == name) {
			type = stackTable[i].Type;
			return (count + i - stackIndex) << 2;
		}
		count += stoi(stackTable[i].op2);
	}
	for (int i = 0; i < globalTable.size(); i++) {
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
		mips_code << "#					define " + name << endl;
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
string loadVarToReg(string name, string no, string regName) {
	mips_code << "				# load " + name + "->" + regName << endl;
	
	if (isNumStr(name)) {
		mips_code << "    li " + regName + " " + name <<  endl;
		return "INT";
	}
	bool neg = false;
	if (name[0] == '-') {
		neg = true;
		name = name.substr(1);
	}
	if (name[0] == '\'') {
		if (neg)
			mips_code << "    subi " + regName + " $0 " + name << endl;
		else
			mips_code << "    li " + regName + " " + name << endl;
		
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
	if (index >= 0) {
		if (isNumStr(no)) {
			mips_code << "    lw " + regName + " -" + to_string(index + stoi(no) * 4) + "($sp)" << endl;
		}
		else {
			loadVarToReg(no, "0", "$t1");
			mips_code << "    sll $t1 $t1 2" << endl;
			mips_code << "    li $t2 " + to_string(index) << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			mips_code << "    sub $t1 $sp $t1" << endl;
			mips_code << "    lw " + regName + " 0($t1)" << endl;
		}
		
	}
	else if (index == -1) {
		mips_code << "    la " << "$t1 " + globalName << endl;
		if (isNumStr(no)) {
			if (type == "INT")
				mips_code << "    lw " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
			else
				mips_code << "    lb " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
		}
		else {
			loadVarToReg(no, "0", "$t2");
			mips_code << "    sll $t2 $t2 2" << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			if (type == "INT")
				mips_code << "    lw " + regName + " 0($t1)" << endl;
			else
				mips_code << "    lb " + regName + " 0($t1)" << endl;
		}
	}
	if (neg)
		mips_code << "    sub " + regName + " $0 " + regName << endl;
	return type;
}

//name->varName
void storeRegToVar(string name, string no,string regName) {
	string globalName, type;
	int index = searchData(name, globalName, type);
	mips_code << "				# store " + regName + "->" + name << endl;
	if (index == -2) {
		dispatch(name, getTempType(name), "0");
		index = searchData(name, globalName, type);
	}
	if (index >= 0) {
		if (isNumStr(no)) {
			mips_code << "    sw " + regName + " -" + to_string(index + stoi(no) * 4) + "($sp)" << endl;
		}
		else {
			loadVarToReg(no, "0", "$t1");
			mips_code << "    sll $t1 $t1 2" << endl;
			mips_code << "    li $t2 " + to_string(index) << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			mips_code << "    sub $t1 $sp $t1" << endl;
			mips_code << "    sw " + regName + " 0($t1)" << endl;
		}
	}
	else if (index == -1) {
		mips_code << "    la " << "$t1 " + globalName << endl;
		if (isNumStr(no)) {
			if (type == "INT")
				mips_code << "    sw " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
			else
				mips_code << "    sb " + regName + " " + to_string(stoi(no) * 4) + "($t1)" << endl;
		}
		else {
			loadVarToReg(no, "0", "$t2");
			mips_code << "    sll $t2 $t2 2" << endl;
			mips_code << "    add $t1 $t1 $t2" << endl;
			if (type == "INT")
				mips_code << "    sw " + regName + " 0($t1)" << endl;
			else
				mips_code << "    sb " + regName + " 0($t1)" << endl;
		}
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
		//iden
		// 1:"'s'"
		// 2:"RET"
		// 3:"12340"
		mips_code << "# print " + midCode.op1 << endl;
		string type = loadVarToReg(midCode.op1, "0", "$a0");
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
	else {
		mips_code << "    la $t0 " + globalName << endl;
		mips_code << "    sw $v0 0($t0)";
	}
}

void calSentence(middleCode midCode) {
	string reg1 = "$t0", reg2 = "$t1", reg3 = "$t0", globalName, type;
	mips_code << "# " + midCode.Type + " " + midCode.op2 + " " + midCode.op3 + "->" + midCode.op1 << endl;
	if (midCode.op2 == "RET" && midCode.op3 == "RET") {//no involve
		if (midCode.Type == "ADD") {
			mips_code << "    add $t0 $v1 $v0" << endl;
		}
		else if (midCode.Type == "SUB") {
			mips_code << "    add $t0 $v1 $v0" << endl;
		}
		else if (midCode.Type == "MUL") {
			mips_code << "    mult $v1 $v0" << endl;
			mips_code << "    mflo $t0" << endl;
		}
		else if (midCode.Type == "DIV") {
			mips_code << "    div $v1 $v0" << endl;
			mips_code << "    mflo $t0" << endl;
		}
		storeRegToVar(midCode.op1, "0", "$t0");
		return;
	}
	else if (midCode.op2 == "RET") {//no involve
		reg1 = "$v0";
		loadVarToReg(midCode.op3, "0", "$t1");
	}
	else if (midCode.op3 == "RET") {//no involve
		loadVarToReg(midCode.op2, "0", "$t0");
		reg2 = "$v0";
	}
	else {
		loadVarToReg(midCode.op2, "0", "$t0");
		loadVarToReg(midCode.op3, "0", "$t1");
	}
	if (midCode.Type == "ADD") {
		mips_code << "    add " + reg3 + " " + reg1 + " " + reg2 << endl;
	}
	else if (midCode.Type == "SUB") {
		mips_code << "    sub " + reg3 + " " + reg1 + " " + reg2 << endl;
	}
	else if (midCode.Type == "MUL") {
		mips_code << "    mult " + reg1 + " " + reg2 << endl;
		mips_code << "    mflo " + reg3 << endl;
	}
	else if (midCode.Type == "DIV") {
		mips_code << "    div " + reg1 + " " + reg2 << endl;
		mips_code << "    mflo " + reg3 << endl;
	}
	storeRegToVar(midCode.op1, "0",reg3);
}

void loadArrSentence(middleCode midCode) {
	mips_code << "# load_arr " + midCode.op2 + "[" + midCode.op3 + "]" + " -> " + midCode.op1 << endl;
	loadVarToReg(midCode.op2, midCode.op3, "$t1");
	string globalName, type;
	if (getTempType(midCode.op1) != "VOID") {
		searchData(midCode.op2, globalName, type);
		dispatch(midCode.op1, type,"0");
	}
	storeRegToVar(midCode.op1, "0", "$t1");
}

void storeArrSentence(middleCode midCode) {
	mips_code << "# store_arr " + midCode.op1 + " -> " + midCode.op2 + "[" + midCode.op3 + "]" << endl;
	loadVarToReg(midCode.op1, "0", "$t3");
	storeRegToVar(midCode.op2, midCode.op3, "$t3");
}

void callFuncSentence(middleCode midCode) {
	int offset = 0;
	for (int i = stackIndex; i < stackTable.size(); i++) {
		offset += stoi(stackTable[i].op2) + 1;
	}
	offset = offset << 2;
	mips_code << "# call function " + midCode.op1 << endl;
	
	mips_code << "    addi $s0 $sp " + to_string(-offset) << endl;
	int i;
	int num;
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
	for (; i < paraList.size(); i++) {
		loadVarToReg(paraList[i].op1, "0", "$t1");
		mips_code << "# para_name:" + paraList[i].op1 << endl;
		int index = -((stoi(paraList[i].op2) - 1) * 4 + 8);
		mips_code << "    sw $t1 " + to_string(index) + "($s0)" << endl;
	}
	while (num--) {
		paraList.pop_back();
	}
	mips_code << "    move $sp $s0" << endl;
	mips_code << "    li $t1 " + to_string(offset) << endl;
	mips_code << "    sw $t1" << " 0($s0)" << endl;
	mips_code << "    jal " + midCode.op1 << endl;
	//mips_code << "    sw  $ra -4($sp)" << endl;
}

void pushParaSentence(middleCode midCode) {
	int remain, all;
	all = stoi(midCode.op3) >> 16;
	remain = stoi(midCode.op3) << 16 >> 16;
	//cout << "all:" + to_string(all) + " index:" + to_string(all - remain) << endl;
	midCode.op2 = to_string(all - remain);
	midCode.op3 = to_string(all);
	paraList.push_back(midCode);
}

void conditionSentence(middleCode midCode) {
	mips_code << "# " + midCode.op2 + " " + midCode.op1 + " " + midCode.op3 << endl;
	loadVarToReg(midCode.op2, "0", "$t0");
	loadVarToReg(midCode.op3, "0", "$t1");
	mips_code << "    sub $t0 $t0 $t1" << endl;
}

void conditionJumpSentence(middleCode midCode) {
	if (condi(">=", "BNZ") || condi("<", "BZ")) {
		mips_code << "    bgez $t0 " + midCode.op1 << endl;
	}
	else if (condi(">", "BNZ") || condi("<=", "BZ")) {
		mips_code << "    bgtz $t0 " + midCode.op1 << endl;
	}
	else if (condi("<=", "BNZ") || condi(">", "BZ")) {
		mips_code << "    blez $t0 " + midCode.op1 << endl;
	}
	else if (condi("<", "BNZ") || condi(">=", "BZ")) {
		mips_code << "    bltz $t0 " + midCode.op1 << endl;
	}
	else if (condi("==", "BNZ") || condi("!=", "BZ")) {
		mips_code << "    beq $t0 $0 " + midCode.op1 << endl;
	}
	else if (condi("!=", "BNZ") || condi("==", "BZ")) {
		mips_code << "    bne $t0 $0 " + midCode.op1 << endl;
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

void sentence(middleCode midCode) {
	if (midCode.Type == "SCAN") {
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
		constDefineSentence(midCode);
	}
	else if (midCode.Type == "VAR") {
		varDefineSentence(midCode);
	}
	else if (midCode.Type == "EXIT") {
		mips_code << "# exit:" << endl;
		mips_code << "# program exit" << endl;
		mips_code << "    li $v0 10" << endl;
		mips_code << "    syscall" << endl;
	}
	else if (midCode.Type == "FUNCHEAD") {
		stackIndex = 0;//reset stack point
		stackTable.clear();
	/* dispatch space for func var */
		dispatch("_offset", "INT", "0");
		dispatch("_ra", "INT", "0");
		vector<middleCode> nameList = funcParaRecord[midCode.op1];
		for (int i = 0; i < nameList.size(); i++) {
			dispatch(nameList[i].op1, nameList[i].Type, "0");
		}
		//store ra
		mips_code << midCode.op1 + ":" << endl;
		mips_code << "    sw $ra -4($sp)" << endl;
		mips_code << "# in func______" << endl;

	}
	else if (midCode.Type == "FUNCRET") {
		if (midCode.op1 != "") {
			loadVarToReg(midCode.op1, "0", "$v0");
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
}

void dotText_gen() {
	mips_code << ".text" << endl;
	bool isGlobal = true;
	int funcbeginIndex = -1,mainbeginIndex = -1;
	// handle main 
	for (int i = 0; i < middleTable.table.size(); i++) {
		if (!isGlobal)
			sentence(middleTable.table[i]);
		else if (middleTable.table[i].Type == "FUNCHEAD" && funcbeginIndex == -1) {
			funcbeginIndex = i;
		}
		else if (middleTable.table[i].Type == "MAINHEAD") {
			mainbeginIndex = i;
			isGlobal = false;
		}
	}
	// handle func
	if (funcbeginIndex != -1) {
		for (int i = funcbeginIndex; i < mainbeginIndex; i++) {
			sentence(middleTable.table[i]);
		}
	}
}

void diffFuncAndExit() {
	int isMain = false;
	vector<middleCode> mid = middleTable.table;
	ofstream midNowFile;
	midNowFile.open("middle.txt");
	for (int i = 0; i < mid.size(); i++) {
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

void translate() {
	diffFuncAndExit();
	mips_code.open("mips.txt");
	dotData_gen();//generate .data
	dotText_gen();//generate .text
	cout << stackIndex << endl;
	for (int i = 0; i < stackTable.size(); i++) {
		cout << stackTable[i].op1 << endl;
	}
}