#include "includeH.h"

middleCode::middleCode(string aType, string aOp1, string aOp2, string aOp3) {
	this->Type = aType;
	this->op1 = aOp1;
	this->op2 = aOp2;
	this->op3 = aOp3;
}

middleCodeTable::middleCodeTable() {
	midFile.open("middle.txt");
}

void middleCodeTable::addDefine(string aType, string aOp1="", string aOp2="", string aOp3="") {
	table.push_back(middleCode(aType, aOp1, aOp2, aOp3));
	midFile <<"    "<<aType<<"    "<<aOp1<<"    "<< aOp2<<"    "<<aOp3 << endl;
}


string middleCodeTable::addTemp(string aType, string aOp2, string aOp3) {
	string now = tempName + to_string(tempCount++);
	table.push_back(middleCode(aType, now, aOp2, aOp3));
	midFile << "    " << aType<<" " << now <<" "<<aOp2<<" " <<aOp3 <<" "<< endl;
	return now;
}

string middleCodeTable::genLabel() {
	string now = labelName + to_string(labelCount++);
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

middleCodeTable middleTable;

