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
	int no;
	middleCode(string aType,string aOp1,string aOp2,string aOp3);
};

class middleCodeTable {
public:
	vector<middleCode> table;
	int tempIntCount = 0;
	int tempCharCount = 0;
	int labelCount = 0;
	const string tempIntName = "INT";
	const string tempCharName = "CHAR";
	const string labelName = "label";
	ofstream midFile;

	middleCodeTable();

	void addDefine(string aType, string aOp1, string aOp2, string aOp3);

	string addIntTemp(string aType, string aOp2, string aOp3);

	string addCharTemp(string aType, string aOp2, string aOp3);

	string genLabel(string type);
};

bool isConst(string ret);

bool isTemp(string str);