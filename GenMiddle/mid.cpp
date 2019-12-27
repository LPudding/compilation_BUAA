#include "includeH.h"

middleCode::middleCode(string aType, string aOp1, string aOp2, string aOp3) {
	this->Type = aType;
	this->op1 = aOp1;
	this->op2 = aOp2;
	this->op3 = aOp3;
}

middleCodeTable::middleCodeTable() {
}

void middleCodeTable::addDefine(string aType, string aOp1="", string aOp2="", string aOp3="") {
	table.push_back(middleCode(aType, aOp1, aOp2, aOp3));
	//midFile <<"    "<<aType<<"    "<<aOp1<<"    "<< aOp2<<"    "<<aOp3 << endl;
}


string middleCodeTable::addIntTemp(string aType, string aOp2, string aOp3) {
	string now = tempIntName + to_string(tempIntCount++);
	table.push_back(middleCode(aType, now, aOp2, aOp3));
	//midFile << "    " << aType<<" " << now <<" "<<aOp2<<" " <<aOp3 <<" "<< endl;
	return now;
}

string middleCodeTable::addCharTemp(string aType, string aOp2, string aOp3) {
	string now = tempCharName + to_string(tempCharCount++);
	table.push_back(middleCode(aType, now, aOp2, aOp3));
	//midFile << "    " << aType << " " << now << " " << aOp2 << " " << aOp3 << " " << endl;
	return now;
}



string middleCodeTable::genLabel(string type) {
	string now = type + to_string(labelCount++);
	return now;
}


bool isConst(string ret) {
	if (ret[0] == '-') {
		return true;
	}
	else if (ret[0] >= '0' && ret[0] <= '9') {
		return true;
	}
	return false;
}

bool isTemp(string str) {
	return regex_match(str, regex("INT[0-9]*")) || regex_match(str, regex("CHAR[0-9]*"));
}

middleCodeTable middleTable;

