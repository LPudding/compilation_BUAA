#pragma once
#include<cstdio>
#include<ctype.h>
#include<cstdlib>
#include<string.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
using namespace std;

class middleCode {
public:
	string Type,op1,op2,op3;
	middleCode(string aType,string aOp1,string aOp2,string aOp3);
};

class middleCodeTable {
public:
	vector<middleCode> table;
	int tempCount = 0;
	int labelCount = 0;
	const string tempName = "LRY";
	const string labelName = "label";
	ofstream midFile;

	middleCodeTable();

	void addDefine(string aType, string aOp1, string aOp2, string aOp3);

	string addTemp(string aType, string aOp2, string aOp3);

	string genLabel();
};

bool isConst(string ret);
