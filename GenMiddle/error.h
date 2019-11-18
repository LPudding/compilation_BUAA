#pragma once
#include<cstdio>
#include<ctype.h>
#include<cstdlib>
#include<string.h>
#include<iostream>
#include<vector>
using namespace std;

#define VAR 0
#define CONST 1
#define ARRAY 2
#define FUNCTION 3
#define PARAMETER 4

/* valueType define */
#define NOEXIST -1
#define VOID 0
#define INT 1
#define CHAR 2

class Term {
public:
	string name;
	int identify;
	int valueType;
	int parameterNum;
	Term(int id, int type);

	void setNum(int num);
};

class symTable {
public:
	int curRange;
	int lastLevelPoint;
	int returnCount = 0;
	int valueType = NOEXIST;
	vector<Term> table;

	symTable();

	bool checkRepeat(int id);

	int checkExist();

	int getId();

	void add(int id, int type);

	void inFunction(int value);

	void setParaNum(int num);

	void outFunction();

	int functionParaType(int index);

	int getFunctionIndex();

	void printTable();

};

void printError(char ch);