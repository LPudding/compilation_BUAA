#include "includeH.h"

const char* keyTable[] = { "IDENFR","INTCON","CHARCON","STRCON","CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK",
"IFTK","ELSETK","DOTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","PLUS","MINU","MULT","DIV","LSS","LEQ",
"GRE","GEQ","EQL","NEQ","ASSIGN","SEMICN","COMMA","LPARENT","RPARENT","LBRACK","RBRACK","LBRACE", "RBRACE"
};

int strSize = 100000, tokenSize = 527, tokenIndex = 0, lineNum = 1;
char token[527],ch;
int k = 0;

extern bool isPrint, enable;
extern char str[];
extern FILE* fl;
void clearToken() {
	tokenIndex = 0;
}

void getChar() {
	ch = str[k];
	k++;
}

void addToken() {
	token[tokenIndex++] = ch;
	if (ch == '\\') 
		token[tokenIndex++] = '\\';
	token[tokenIndex] = '\0';
	//printf("%c %c\n",token[tokenIndex-1],ch);
}

void finishToken() {
	token[tokenIndex] = '\0';
}

int isunderline(char c) {
	if (c == '_') {
		return 1;
	}
	else {
		return 0;
	}
}

void back() {
	k--;
}

int matcher() {
	if (strcmp(token, "const") == 0) {
		return 4;
	}
	else if (strcmp(token, "int") == 0) {
		return 5;
	}
	else if (strcmp(token, "char") == 0) {
		return 6;
	}
	else if (strcmp(token, "void") == 0) {
		return 7;
	}
	else if (strcmp(token, "main") == 0) {
		return 8;
	}
	else if (strcmp(token, "if") == 0) {
		return 9;
	}
	else if (strcmp(token, "else") == 0) {
		return 10;
	}
	else if (strcmp(token, "do") == 0) {
		return 11;
	}
	else if (strcmp(token, "while") == 0) {
		return 12;
	}
	else if (strcmp(token, "for") == 0) {
		return 13;
	}
	else if (strcmp(token, "scanf") == 0) {
		return 14;
	}
	else if (strcmp(token, "printf") == 0) {
		return 15;
	}
	else if (strcmp(token, "return") == 0) {
		return 16;
	}
	else {
		return 0;
	}
}


void newLine() {
	lineNum++;
	isPrint = false;
}

bool isChar(char ch) {
	if (ch == '+' || ch == '-')
		return true;
	if (ch == '*' || ch == '/')
		return true;
	if (isalpha(ch) || isdigit(ch) || ch == '_')
		return true;
	return false;
}

bool isStr(char ch) {
	if (ch == 34)
		return false;
	if (ch >= 32 && ch <= 126)
		return true;
	return false;
}

int getsym() {
	int result;
	clearToken();
	getChar();
	while (isspace(ch)) {
		if (ch == '\n')
			newLine();
		getChar();
	}
	if (isalpha(ch) || isunderline(ch)) {
		while (isalpha(ch) || isdigit(ch) || isunderline(ch)) {
			addToken();
			getChar();
			//printf("%s\n",token);
		}
		finishToken();
		back();
		result = matcher();
	}
	else if (isdigit(ch)) {
		while (isdigit(ch)) {
			addToken();
			getChar();
		}
		finishToken();
		back();
		result = 1;
	}
	else if (ch == '\'') {
		int num = 0;
		getChar();
		while (ch != '\'' && num == 0) {
			num++;
			addToken();
			if (!isChar(ch)) {
				printError('a');
			}
			getChar();
		}
		if (ch != '\'') {
			printError('a');
			k--;
		}
		finishToken();
		result = 2;
	}
	else if (ch == '\"') {
		getChar();
		while (ch != '\"') {
			addToken();
			if (ch == '\n') {
				printError('a');
				newLine();
				return -1;
			}
			if (!isStr(ch)) {
				printError('a');
			}
			getChar();
		}
		finishToken();
		result = 3;
	}
	else if (ch == '+') {
		addToken();
		finishToken();
		result = 17;
	}
	else if (ch == '-') {
		addToken();
		finishToken();
		result = 18;
	}
	else if (ch == '*') {
		addToken();
		finishToken();
		result = 19;
	}
	else if (ch == '/') {
		addToken();
		finishToken();
		result = 20;
	}
	else if (ch == '<') {
		addToken();
		getChar();
		if (ch == '=') {
			addToken();
			result = 22;
		}
		else {
			back();
			result = 21;
		}
		finishToken();
	}
	else if (ch == '>') {
		addToken();
		getChar();
		if (ch == '=') {
			addToken();
			result = 24;
		}
		else {
			back();
			result = 23;
		}
		finishToken();
	}
	else if (ch == '=') {
		addToken();
		getChar();
		if (ch == '=') {
			addToken();
			result = 25;
		}
		else {
			back();
			result = 27;
		}
		finishToken();
	}
	else if (ch == '!') {
		addToken();
		getChar();
		if (ch == '=') {
			addToken();
			result = 26;
		}
		else {
			result = -1;
		}
		finishToken();
	}
	else if (ch == ';') {
		addToken();
		result = 28;
		finishToken();
	}
	else if (ch == ',') {
		addToken();
		result = 29;
		finishToken();
	}
	else if (ch == '(') {
		addToken();
		result = 30;
		finishToken();
	}
	else if (ch == ')') {
		addToken();
		result = 31;
		finishToken();
	}
	else if (ch == '[') {
		addToken();
		result = 32;
		finishToken();
	}
	else if (ch == ']') {
		addToken();
		result = 33;
		finishToken();
	}
	else if (ch == '{') {
		addToken();
		result = 34;
		finishToken();
	}
	else if (ch == '}') {
		addToken();
		result = 35;
		finishToken();
	}
	else {
		result = 0;
		printError('a');
	}
	return result;
}

/* glimpse next word */  //watch getsym 
int watch() {
	bool is = isPrint;
	enable = false;
	int i = k, num = lineNum, id = getsym();
	k = i;
	lineNum = num;
	isPrint = is;
	enable = true;
	return id;
}

int watchTwice() {
	bool is = isPrint;
	enable = false;
	int i = k, num = lineNum, id = getsym();
	id = getsym();
	k = i;
	lineNum = num;
	isPrint = is;
	enable = true;
	return id;
}

int watchThrice() {
	enable = false;
	bool is = isPrint;
	int i = k, num = lineNum, id = getsym();
	id = getsym();
	id = getsym();
	k = i;
	lineNum = num;
	isPrint = is;
	enable = true;
	return id;
}

void printWord(int id) {
	printf("%s %s\n", keyTable[id], token);
	fprintf(fl, "%s %s\n", keyTable[id], token);
}