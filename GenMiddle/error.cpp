#include "includeH.h"

bool isPrint = false, enable = true;
int lastFuncCallIndex;
extern int lineNum;
extern char token[];


	Term::Term(int id, int type) {
		identify = id;
		valueType = type;
		name = token;
	}

	void Term::setNum(int num) {
		parameterNum = num;
	}


	symTable::symTable() {
		curRange = 0;
		lastLevelPoint = 0;
	}

	bool symTable::checkRepeat(int id) {//func 屏蔽参数   //VAR、CONST、ARRAY、PARAMETER 同层所有数，不屏蔽参数
		bool isIgnorePara = false;
		int begin = 0;
		if (id == FUNCTION) {
			isIgnorePara = true;
			begin = 0;
		}
		else if (curRange) {
			begin = lastLevelPoint;
		}
		for (; begin < table.size(); begin++) {
			if (table[begin].identify == PARAMETER && isIgnorePara)
				continue;
			if (table[begin].name == token)
				return false;
		}
		return true;
	}

	int symTable::checkExist() {// return type
		int state = 0;
		for (int begin = table.size() - 1; begin != -1; begin--) {
			if (state == 1) {
				if (table[begin].identify == PARAMETER) {
					if (table[begin].name == token) {
						return table[begin].valueType;
					}
				}
				else
					state = 2;
			}
			if (curRange && state == 0 && table[begin].identify == PARAMETER) {
				state = 1;
				if (table[begin].name == token)
					return table[begin].valueType;
			}
			if (state == 2 || state == 0) {
				if (table[begin].name == token && table[begin].identify != PARAMETER)
					return table[begin].valueType;
			}
		}
		return NOEXIST;//no exist
	}

	int symTable::getId() {
		int state = 0;
		for (int begin = table.size() - 1; begin != -1; begin--) {
			if (state == 1) {
				if (table[begin].identify == PARAMETER) {
					if (table[begin].name == token) {
						return table[begin].identify;
					}
				}
				else
					state = 2;
			}
			if (curRange && state == 0 && table[begin].identify == PARAMETER) {
				state = 1;
				if (table[begin].name == token)
					return table[begin].identify;
			}
			if (state == 2 || state == 0) {
				if (table[begin].name == token && table[begin].identify != PARAMETER)
					return table[begin].identify;
			}
		}
		return NOEXIST;//no exist
	}

	void symTable::add(int id, int type) {
		if (checkRepeat(id)) {
			table.push_back(Term(id, type));
		}

		else
			printError('b');
	}

	void symTable::inFunction(int value) {
		curRange++;
		lastLevelPoint = table.size(); //record function index
		returnCount = 0;
		valueType = value;
	}

	void symTable::setParaNum(int num) {
		table[lastLevelPoint].setNum(num);
	}

	void symTable::outFunction() {
		curRange--;
		int size = table.size() - lastLevelPoint - 1 - table[lastLevelPoint].parameterNum;
		while (size--) {
			table.pop_back();
		}
		if (valueType != VOID && returnCount == 0)
			printError('h');
		returnCount = 0;
		valueType = NOEXIST;
	}

	int symTable::functionParaType(int index) {
		return table[index + lastFuncCallIndex].valueType;
	}

	int symTable::getFunctionIndex() {
		for (int i = 0; i < table.size(); i++)
			if (table[i].identify == FUNCTION && table[i].name == token)
				return i;
		return -1;
	}

	void symTable::printTable() {
		for (int i = 0; i < table.size(); i++) {
			cout << "id:" << table[i].identify << "     value:" << table[i].valueType << "     name:" << table[i].name << endl;
		}
	}

symTable sym_table;


void printError(char ch) {
	if (!isPrint && enable) {
		printf("%d %c\n", lineNum, ch);
		fprintf(fe, "%d %c\n", lineNum, ch);
		isPrint = true;
	}
}