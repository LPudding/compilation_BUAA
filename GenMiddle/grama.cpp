#include "includeH.h"
using namespace std;
#define debug 1 // print some gramma info



/* lexical analysis */
extern middleCodeTable middleTable;
extern char str[];
extern char token[];
extern symTable sym_table;
extern int lastFuncCallIndex,lineNum;
extern FILE* fc;
//func 
//

char* func[1000];
int funcLength = 0;
char* noFunc[1000];
int noFuncLength = 0;

int v0_used = 0; 
//been use:0 
//wait to be used:1

map<string, vector<string>> funcRecord;

void constDefine() {
	int id = watch();
	if (id == 5) {
		printWord(getsym());// int
		do {
			printWord(getsym()); //identify
			string iden(token);
			sym_table.add(CONST, INT);
			id = watch();
			if (id == 27) {
				int symbol; //+:1 -:0
				printWord(getsym()); // =
				id = watch();
				if (id == 17 || id == 18) {
					symbol = (id == 17) ? 1 : 0;
					getsym(); // + -
					printWord(id);
					id = getsym();
					printWord(id); // number
					string num(token);
					if (!symbol)
						num = "-" + num;
					middleTable.addDefine("CONST", iden, "INT", num);
					if (debug) {
						fprintf(fc, "<无符号整数>\n");
						fprintf(fc, "<整数>\n");
					}

				}
				else if (id == 1) {
					id = getsym();
					printWord(id); // number
					string num(token);
					middleTable.addDefine("CONST", iden, "INT", num);
					if (debug) {
						fprintf(fc, "<无符号整数>\n");
						fprintf(fc, "<整数>\n");
					}
				}
				else {
					printError('o');
					getsym();
				}

				id = watch();
			}
			if (id == 29) {
				printWord(getsym());
			}
		} while (id == 29);
	}
	else if (id == 6) { //char
		printWord(getsym());
		if (watch() == 0) {
			printWord(getsym()); //identify
			sym_table.add(CONST, CHAR);
		}
		string iden(token); // record name
		if (watch() == 27)
			printWord(getsym()); //=
		if (watch() == 2) {
			printWord(getsym()); //char
			string alpha(token);
			middleTable.addDefine("CONST", iden, "CHAR", alpha);
		}
		else {
			printError('o');
			getsym();
		}
		id = watch();
		while (id == 29) {
			printWord(getsym());
			printWord(getsym()); //identify
			string iden(token);
			sym_table.add(CONST, CHAR);
			printWord(getsym()); //=
			if (watch() == 2) {
				printWord(getsym()); //char
				string alpha(token);
				middleTable.addDefine("CONST", iden, "CHAR", alpha);
			}
			else {
				printError('o');
				getsym();
			}
			id = watch();
		}
	}
	if (debug)
		fprintf(fc, "<常量定义>\n");
}

void constState() {
	int id = watch();
	while (id == 4) {
		id = getsym();
		printWord(id); // const
		constDefine();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else {
			printError('k');
		}
		id = watch();
		//fprintf(fc, "%d\n", id);
	}
	if (debug)
		fprintf(fc, "<常量说明>\n");
}

void varDefine() {
	int id = watch();
	string iden, count;
	if (id == 5) {
		printWord(getsym()); // int
		printWord(getsym()); // identify
		iden = token;
		sym_table.add(VAR, INT);
		id = watch();
		if (id == 32) {
			printWord(getsym());//[
			printWord(getsym());
			count = token;
			if (debug)
				fprintf(fc, "<无符号整数>\n");
			if (watch() == 33)
				printWord(getsym());//]
			else
				printError('m');
		}
		else
			count = "0";
		middleTable.addDefine("VAR", iden, "INT", count);
		id = watch();
		while (id == 29) {
			printWord(getsym());//,
			printWord(getsym()); // identify
			iden = token;
			sym_table.add(VAR, INT);
			id = watch();
			if (id == 32) {
				printWord(getsym());//[
				printWord(getsym());
				count = token;
				if (debug)
					fprintf(fc, "<无符号整数>\n");
				if (watch() == 33)
					printWord(getsym());//]
				else
					printError('m');
			}
			else
				count = "0";
			middleTable.addDefine("VAR", iden, "INT", count);
			id = watch();
		}
	}
	else if (id == 6) {
		printWord(getsym()); // char
		printWord(getsym()); // identify
		iden = token;
		sym_table.add(VAR, CHAR);
		id = watch();
		if (id == 32) {
			printWord(getsym());//[
			printWord(getsym());
			count = token;
			if (debug)
				fprintf(fc, "<无符号整数>\n");
			printWord(getsym());//]
		}
		else
			count = "0";
		middleTable.addDefine("VAR", iden, "CHAR", count);
		id = watch();
		while (id == 29) {
			printWord(getsym());//,
			printWord(getsym()); // identify
			iden = token;
			sym_table.add(VAR, CHAR);
			id = watch();
			if (id == 32) {
				printWord(getsym());//[
				printWord(getsym());
				count = token;
				if (debug)
					fprintf(fc, "<无符号整数>\n");
				printWord(getsym());//]
			}
			else
				count = "0";
			middleTable.addDefine("VAR", iden, "CHAR", count);
			id = watch();
		}
	}
	if (debug)
		fprintf(fc, "<变量定义>\n");
}

void varState() {
	int id = watch();
	while (id == 5 || id == 6) {
		varDefine();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
		if (watchThrice() == 30) {
			break;
		}
		id = watch();
	}
	if (debug)
		fprintf(fc, "<变量说明>\n");

}

int statementClassifer() {
	int id;
	id = watch();
	if (id == 28) {
		return EMPTY;
	}
	// watch first string
	if (strcmp("if", token) == 0) {
		return CONDITION;
	}
	else if (strcmp("while", token) == 0 || strcmp("do", token) == 0 || strcmp("for", token) == 0) {
		return LOOP;
	}
	else if (id == 34) {
		return COLUMN;
	}
	else if (strcmp("scanf", token) == 0) {
		return SCANF;
	}
	else if (strcmp("printf", token) == 0) {
		return PRINTF;
	}
	else if (strcmp("return", token) == 0) {
		return FUNCRETURN;
	}
	//watch second string
	id = watch();
	if (id == 0) {
		if (watchTwice() == 30) {
			watch();
			if (sym_table.checkExist() == VOID)
				return NORETURN;
			return RETURN;
		}
		else if (watchTwice() == 27 || watchTwice() == 32) {
			return ASSIGN;
		}
	}
	return -1;
}

void valueTable(string funcName) {
	string ret;
	int id = watch(), index = 0, type, num, enable = 1;
	if (lastFuncCallIndex == -1) {
		enable = 0;
		num = 0;
	}
	else {
		num = sym_table.table[lastFuncCallIndex].parameterNum;
	}
	if (id == 31) {//empty
		if (enable && num != 0)
			printError('d');
		if (debug)
			fprintf(fc, "<值参数表>\n");
		return;
	}
	index++;
	type = expression(ret);
	middleTable.addDefine("PUSH_PARA", ret, funcName, to_string((num << 16) + num - index));
	if (enable && index <= num && type != sym_table.functionParaType(index))
		printError('e');
	id = watch();

	while (id == 29) { //,
		printWord(getsym());
		index++;
		type = expression(ret);
		middleTable.addDefine("PUSH_PARA", ret, funcName, to_string((num << 16) + num - index));
		if (enable && index <= num && type != sym_table.functionParaType(index))
			printError('e');
		id = watch();
	}
	if (enable && num != index)
		printError('d');
	else
		middleTable.addDefine("CALL_FUNC", funcName, "", "");
	if (debug)
		fprintf(fc, "<值参数表>\n");
}

void returnFuncStatement() {
	int id = watch();
	string funcName;
	if (id == 0) {//identify
		funcName = token;
		lastFuncCallIndex = sym_table.getFunctionIndex();
		printWord(getsym());
		//middleTable.addDefine("CALL_FUNC", token, "", "");
		if (sym_table.checkExist() == NOEXIST)
			printError('c');
		id = watch();
		//fprintf(fc, "id:%d\n", id);
		if (id == 30) {//(
			printWord(getsym());
			valueTable(funcName);
			id = watch();
			if (id == 31) //)
				printWord(getsym());
		}
	}
	if (debug)
		fprintf(fc, "<有返回值函数调用语句>\n");
}

void noReturnFuncStatement() {
	int id = watch();
	string funcName;
	if (id == 0) {//identify
		lastFuncCallIndex = sym_table.getFunctionIndex();
		funcName = token;
		printWord(getsym());
		//middleTable.addDefine("CALL_FUNC", token, "", "");
		if (sym_table.checkExist() == NOEXIST)
			printError('c');
		id = watch();
		if (id == 30) {//(
			printWord(getsym());
			valueTable(funcName);
			id = watch();
			if (id == 31) {
				printWord(getsym());
			}
		}
	}
	if (debug)
		fprintf(fc, "<无返回值函数调用语句>\n");
}

int factor(string& name) {//may be a question
	int flag = 0, id = watch(), type = INT, arrayType;
	if (id == 1 || id == 17 || id == 18) { // number
		if (id != 1)
			printWord(getsym());
		if (id == 18)
			name = "-";
		printWord(getsym());
		name += token; //return a const
		if (debug) {
			fprintf(fc, "<无符号整数>\n");
			fprintf(fc, "<整数>\n");
		}
		type = INT;
	}
	else if (id == 2) { // char 
		printWord(getsym());
		name = token;//return a const
		name = "\'" + name + "\'";
		type = CHAR;
	}
	else if (id == 0) {
		if (sym_table.getId() == FUNCTION && (sym_table.checkExist() == INT || sym_table.checkExist() == CHAR))
			flag = 1;
		if (!flag) {
			if (sym_table.checkExist() == NOEXIST && watchTwice() == 30) {
				printError('c');
				returnFuncStatement();
				if (debug)
					fprintf(fc, "<因子>\n");
				return NOEXIST;
			}
			printWord(getsym());
			name = token;//record id name
			type = sym_table.checkExist();
			if (type == NOEXIST)
				printError('c');
			id = watch();
			if (id == 32) {
				string ind;
				printWord(getsym());// [
				arrayType = expression(ind); //expression
				name = middleTable.addTemp("LOAD_ARR", name, ind);
				if (arrayType != INT)
					printError('i');
				id = watch();
				if (id == 33) {
					printWord(getsym()); // ]
				}
				else
					printError('m');
			}
		}
		else {
			type = sym_table.checkExist();
			if (v0_used) {
				middleTable.addDefine("MOV","$v1","$v0","");
				v0_used = 0;
			}
			returnFuncStatement();
			name = "RET";
		}
	}
	else if (id == 30) { // (
		type = INT;
		printWord(getsym());
		expression(name); // express
		id = watch();
		if (id == 31) {
			printWord(getsym()); // )
		}
		else
			printError('l');
	}
	if (debug)
		fprintf(fc, "<因子>\n");
	return type;
}

int term(string& name) {
	string ret;
	int id, type = factor(ret);
	if (ret == "RET") {
		v0_used = 1;
	}
	id = watch();
	while (id == 19 || id == 20) {
		string ret1;
		printWord(getsym());
		factor(ret1);
		ret = middleTable.addTemp(id == 19 ? "MUL" : "DIV", ret, ret1);
		id = watch();
		type = INT;
	}
	name = ret;
	v0_used = 0;
	if (debug)
		fprintf(fc, "<项>\n");
	return type;
}

int expression(string& name) {
	string ret;
	bool neg = false;
	int id = watch(), type = 0;
	/* +/- */
	if (id == 17 || id == 18) {
		id = watch();
		type = INT;
		if (id == 18) {
			if (watchTwice() == 1 && (watchThrice() == 18 || watchThrice() == 19))
				neg = false;
			else {
				neg = true;
				printWord(getsym());
			}
		}
		else {
			printWord(getsym());
		}
	}

	/* term */
	if (type == 0)
		type = term(ret);
	else
		term(ret);
	if (ret == "RET") {
		v0_used = 1;
	}
	if (neg) {
		if (ret[0] == '-')
			ret = ret.substr(1);
		else
			ret = "-" + ret;
	}
	/* +/- term*/
	id = watch();
	while (id == 17 || id == 18) {
		string ret1;
		printWord(getsym());
		term(ret1);
		ret = middleTable.addTemp(id == 17 ? "ADD" : "SUB", ret, ret1);
		id = watch();
		type = INT;
	}
	name = ret;
	if (debug)
		fprintf(fc, "<表达式>\n");
	v0_used = 0;
	return type;
}

void condition(string& condi) {
	int id, type1, type2 = VOID;
	string condi1, condi2;
	type1 = expression(condi1);
	id = watch();
	if (id >= 21 && id <= 26) {
		string op = token;
		printWord(getsym());
		type2 = expression(condi2);
		middleTable.addDefine("CONDITION", op, condi1, condi2);
		condi = op;
	}
	else {
		middleTable.addDefine("CONDITION", "!=", condi1, "0");
		condi = "!=";
	}
	if (type1 != INT)
		printError('f');
	else if (type2 == CHAR)
		printError('f');
	/* end */
	if (debug)
		fprintf(fc, "<条件>\n");
}

void loopStatement() {
	int id = watch(), assignId;
	if (id == 12) {//while
		printWord(getsym());
		id = watch();
		if (id == 30) { // (
			printWord(getsym());
		}
		string label1 = middleTable.genLabel("while_Begin");
		middleTable.addDefine("LABEL", label1, "", "");
		string condi;
		condition(condi); //condition
		id = watch();
		if (id == 31) {// )
			printWord(getsym());
			string label2 = middleTable.genLabel("while_End");
			middleTable.addDefine("BZ", label2, condi, "");
			statement();//statement
			middleTable.addDefine("GOTO", label1, "", "");
			middleTable.addDefine("LABEL", label2, "", "");
		}
		else
			printError('l');
	}
	else if (id == 11) {//do
		printWord(getsym());
		string label1 = middleTable.genLabel("do_while_Begin");
		middleTable.addDefine("LABEL", label1, "", "");
		statement();//statement
		id = watch();
		if (id == 12) { //while
			printWord(getsym());
		}
		else
			printError('n');
		id = watch();
		if (id == 30) {//(
			printWord(getsym());
		}
		string condi;
		condition(condi);
		id = watch();
		if (id == 31) {//)
			printWord(getsym());
		}
		else
			printError('l');
		middleTable.addDefine("BNZ", label1, condi, "");
	}
	else if (id == 13) {//for
		printWord(getsym());
		id = watch();
		if (id == 30) {// (
			printWord(getsym());
			id = watch();
			if (id == 0) {
				printWord(getsym()); // identify
				string iden = token;
				if (sym_table.checkExist() == NOEXIST)
					printError('c');
				assignId = sym_table.getId();
				if (assignId == CONST)//can't assign to a CONST
					printError('j');
				id = watch();
				if (id == 27) {// =
					printWord(getsym());
				}

				string ret;
				expression(ret); // expression
				middleTable.addDefine("ADD", iden, ret, "0");
				id = watch();
				if (id == 28) {//;
					printWord(getsym());
				}
				else
					printError('k');
				string label1 = middleTable.genLabel("for_Begin");
				string label2 = middleTable.genLabel("for_End");
				middleTable.addDefine("LABEL", label1, "", "");
				string condi;
				condition(condi);//condition
				middleTable.addDefine("BZ", label2, condi, "");
				id = watch();
				if (id == 28) {//;
					printWord(getsym());
				}
				else
					printError('k');
				id = watch();
				string op, op1, op2, op3;
				if (id == 0) {//identify
					printWord(getsym());
					op1 = token;
					if (sym_table.checkExist() == NOEXIST)
						printError('c');
				}
				id = watch();
				if (id == 27) {//=
					printWord(getsym());
				}
				id = watch();
				if (id == 0) {//identify
					printWord(getsym());
					op2 = token;
					assignId = sym_table.getId();
					if (assignId == CONST)//can't assign to a CONST
						printError('j');
					if (sym_table.checkExist() == NOEXIST)
						printError('c');
				}
				id = watch();
				if (id == 17 || id == 18) {// + -
					printWord(getsym());
					op = (id == 17) ? "ADD" : "SUB";
				}
				id = watch();
				if (id == 1) {//number
					printWord(getsym());
					op3 = token;
					if (debug) {
						fprintf(fc, "<无符号整数>\n");
						fprintf(fc, "<步长>\n");
					}
				}
				id = watch();
				if (id == 31) {
					printWord(getsym());
				}
				else
					printError('l');
				statement();
				middleTable.addDefine(op, op1, op2, op3);
				middleTable.addDefine("GOTO", label1, "", "");
				middleTable.addDefine("LABEL", label2, "", "");
			}
		}
	}
	if (debug)
		fprintf(fc, "<循环语句>\n");
}

void conditionStatement() {
	int id = watch();
	if (id == 9) { //if
		printWord(getsym());
	}
	id = watch();
	if (id == 30) { // (
		printWord(getsym());
	}
	string condi;
	condition(condi);
	id = watch();
	if (id == 31) { // )
		printWord(getsym());
	}
	else
		printError('l');
	string label1 = middleTable.genLabel("if");
	middleTable.addDefine("BZ", label1, condi, "not_If");
	statement();
	id = watch();
	if (id == 10) {
		string label2 = middleTable.genLabel("if_End");
		middleTable.addDefine("GOTO", label2, "", "");
		printWord(getsym());
		statement();
		middleTable.addDefine("LABEL", label2, "", "");
	}
	middleTable.addDefine("LABEL", label1, "", "");
	if (debug)
		fprintf(fc, "<条件语句>\n");
}

void scanfStatement() {
	int id = watch();
	if (id == 14) {//scan
		printWord(getsym());
	}
	id = watch();
	if (id == 30) { // (
		printWord(getsym());
	}
	id = watch();
	if (id == 0) { //identify
		middleTable.addDefine("SCAN", token, "", "");
		printWord(getsym());
		if (sym_table.checkExist() == NOEXIST)
			printError('c');
	}
	id = watch();
	while (id == 29) { //,
		printWord(getsym());
		id = watch();
		if (id == 0) { //identify
			printWord(getsym());
			middleTable.addDefine("SCAN", token, "", "");
			if (sym_table.checkExist() == NOEXIST)
				printError('c');
			id = watch();
		}
	}
	id = watch();
	if (id == 31) {//)
		printWord(getsym());
	}
	else
		printError('l');
	if (debug)
		fprintf(fc, "<读语句>\n");
}

void printfStatement() { //string may be a question
	int id = watch();
	if (id == 15) {//print
		printWord(getsym());
	}
	id = watch();
	if (id == 30) { //(
		printWord(getsym());
	}
	id = watch();
	if (watch() == -1) {
		getsym();
		return;
	}
	if (watch() == 3 && watchTwice() == 29) {//string ,
		printWord(getsym());
		middleTable.addDefine("PRINT", token, "0", "0");
		if (debug)
			fprintf(fc, "<字符串>\n");
		printWord(getsym());
		string rec;
		int type = expression(rec);
		middleTable.addDefine("PRINT", rec, "1", type ? "INT" : "CHAR");
	}
	else if (watch() == 3 && watchTwice() == 31) {//string )
		printWord(getsym());
		middleTable.addDefine("PRINT", token, "0", "");
		if (debug)
			fprintf(fc, "<字符串>\n");
	}
	else {
		//fprintf(fc, "before\n");
		string rec;
		int type = expression(rec);
		middleTable.addDefine("PRINT", rec, "1", type ? "INT" : "CHAR");
	}
	id = watch();
	if (id == 31) {//)
		printWord(getsym());
	}
	else
		printError('l');
	if (debug)
		fprintf(fc, "<写语句>\n");
}

void assignStatement() {
	int id = watch(), type, assignId = sym_table.getId();
	lineNum++;
	if (assignId == CONST)//can't assign to a CONST
		printError('j');
	lineNum--;
	if (id == 0) {//identify [
		printWord(getsym());
		string iden = token, ind = "";
		if (sym_table.checkExist() == NOEXIST)
			printError('c');
		id = watch();
		if (watch() == 32) { //[
			printWord(getsym());
			type = expression(ind);//expression
			if (type != INT)
				printError('i');
			id = watch();
			if (id == 33)  //]
				printWord(getsym());
			else
				printError('m');
		}
		// = expression
		id = watch();
		if (id == 27) { //=
			printWord(getsym());
			string rec, op;
			expression(rec);
			if (ind == "") {
				middleTable.addDefine("ADD", iden, rec, "0");
			}
			else {
				middleTable.addDefine("STORE_ARR", rec, iden, ind);
			}

		}
	}
	if (debug)
		fprintf(fc, "<赋值语句>\n");
}

void funcReturnStatement() {
	int id = watch(), type;
	if (id == 16) { //return
		printWord(getsym());
		id = watch();
		/*if (sym_table.valueType == VOID)
			printError('g');*/
		if (id == 30) { //(
			if (sym_table.valueType == VOID)
				printError('g');
			sym_table.returnCount++;
			printWord(getsym());
			string rec;
			type = expression(rec);
			middleTable.addDefine("FUNCRET", rec, "", "");
			if (sym_table.valueType != type)
				printError('h');
			id = watch();
			if (id == 31) {
				printWord(getsym());
			}
			else
				printError('l');
		}
		else {
			middleTable.addDefine("FUNCRET", "", "", "");
		}
	}
	if (debug)
		fprintf(fc, "<返回语句>\n");
}

void statement() {
	watch();
	int type = statementClassifer(), id;
	watch();
	if (type == CONDITION) {
		conditionStatement();
	}
	else if (type == LOOP) {
		loopStatement();
	}
	else if (type == COLUMN) {
		id = watch();
		if (id == 34) {
			printWord(getsym());
			statementColumn();
			id = watch();
			if (id == 35) {
				printWord(getsym());
			}
		}
	}
	else if (type == SCANF) {
		scanfStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == PRINTF) {
		printfStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == RETURN) {
		returnFuncStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == NORETURN) {
		noReturnFuncStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == ASSIGN) {
		assignStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == FUNCRETURN) {
		funcReturnStatement();
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}
	else if (type == EMPTY) {
		id = watch();
		if (id == 28) {
			printWord(getsym());
		}
		else
			printError('k');
	}

	/* end */
	if (debug)
		fprintf(fc, "<语句>\n");
}

void statementColumn() {
	while (statementClassifer() != -1) {
		statement();
	}
	/* end */
	if (debug)
		fprintf(fc, "<语句列>\n");
}

void compoundStatement() {
	int id = watch();
	/* cons and var*/
	if (id == 4) {
		constState();
	}
	id = watch();
	if (id == 5 || id == 6) {
		varState();
	}
	/* statement column*/
	statementColumn();
	/* end*/
	if (debug)
		fprintf(fc, "<复合语句>\n");

}

void recordFunc() { //record function name
	func[funcLength] = (char*)malloc(strlen(token) + 1);
	strcpy(func[funcLength++], token);
}

void recordNoFunc() {
	noFunc[noFuncLength] = (char*)malloc(strlen(token) + 1);
	strcpy(noFunc[noFuncLength++], token);
}

void returnFunc() {
	int id = watch(), lastId, num = 0;
	string iden;
	vector<string> nameList;//record paraName
	if (id == 5 || id == 6) {
		printWord(getsym());// int/char
		lastId = id;
		id = watch();
		if (id == 0) {
			if (lastId == 5) {
				iden = token;
				middleTable.addDefine("FUNC", iden, "INT", "");
				sym_table.inFunction(INT); // record a function 
				sym_table.add(FUNCTION, INT);
			}
			else {
				iden = token;
				middleTable.addDefine("FUNC", iden, "CHAR", "");
				sym_table.inFunction(CHAR); // record a function 
				sym_table.add(FUNCTION, CHAR);
			}
			printWord(getsym());// identify
			//recordFunc();
			if (debug)
				fprintf(fc, "<声明头部>\n");
		}
		id = watch();   //para
		if (id == 30) {//(
			printWord(getsym());
			/**/
			id = watch();
			if (id == 5 || id == 6) {
				lastId = id;
				printWord(getsym());
				id = watch();
				if (id == 0) {
					printWord(getsym());
					if (lastId == 5) {
						string iden(token);
						middleTable.addDefine("PARA", iden, "INT", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, INT);
					}
					else {
						string iden(token);
						middleTable.addDefine("PARA", iden, "CHAR", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, CHAR);
					}
					num++;
				}
			}
			id = watch();
			/*, int/char indentify*/
			while (id == 29) {
				printWord(getsym());
				id = watch();

				if (id == 5 || id == 6) {
					printWord(getsym());
				}
				lastId = id;
				id = watch();
				if (id == 0) {
					printWord(getsym());
					if (lastId == 5) {
						string iden(token);
						middleTable.addDefine("PARA", iden, "INT", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, INT);
					}
					else {
						string iden(token);
						middleTable.addDefine("PARA", iden, "CHAR", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, CHAR);
					}
					num++;
				}
				id = watch();
			}
			sym_table.setParaNum(num);
			funcRecord[iden] = nameList;
			/**/
			if (debug)
				fprintf(fc, "<参数表>\n");
			id = watch();
			if (id == 31)  //)
				printWord(getsym());
			else
				printError('l');
			id = watch();
			middleTable.addDefine("FUNCHEAD", iden, "", "");
			if (id == 34)  //{
				printWord(getsym());
			/**/
			compoundStatement();
			/**/
			id = watch();
			if (id == 35) //}
				printWord(getsym());
			sym_table.outFunction();
			//middleTable.addDefine("FUNCRET", "", "", "");
			middleTable.addDefine("FUNCTAIL", "", "", "");

		}
	}
	if (debug)
		fprintf(fc, "<有返回值函数定义>\n");
}

void noReturnFunc() {
	int id = watch(), lastId, num = 0;
	string iden;
	vector<string> nameList;//record paraName
	if (id == 7) {
		printWord(getsym());// void
		id = watch();
		if (id == 0) {
			printWord(getsym());// identify
			iden = token;
			sym_table.inFunction(VOID);
			sym_table.add(FUNCTION, VOID);
			//recordNoFunc();
		}
		id = watch();   //para
		if (id == 30) {//(
			printWord(getsym());
			/**/
			id = watch();
			if (id == 5 || id == 6) {
				printWord(getsym());
				lastId = id;
				id = watch();
				if (id == 0) {
					num++;
					if (lastId == 5) {
						string iden(token);
						middleTable.addDefine("PARA", iden, "INT", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, INT);
					}
					else {
						string iden(token);
						middleTable.addDefine("PARA", iden, "CHAR", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, CHAR);
					}
					printWord(getsym());
				}
			}
			id = watch();
			/*, int/char indentify*/
			while (id == 29) {
				printWord(getsym());
				id = watch();
				if (id == 5 || id == 6) {
					printWord(getsym());
				}
				lastId = id;
				id = watch();
				if (id == 0) {
					num++;
					if (lastId == 5) {
						string iden(token);
						middleTable.addDefine("PARA", iden, "INT", "");
						nameList.push_back(iden);
						sym_table.add(PARAMETER, INT);
					}
					else {
						sym_table.add(PARAMETER, CHAR);
						string iden(token);
						nameList.push_back(iden);
						middleTable.addDefine("PARA", iden, "CHAR", "");

					}
					printWord(getsym());
				}
				id = watch();
			}
			sym_table.setParaNum(num);
			/**/
			if (debug)
				fprintf(fc, "<参数表>\n");
			id = watch();
			if (id == 31) { //)
				printWord(getsym());
			}
			else
				printError('l');
			id = watch();
			middleTable.addDefine("FUNCHEAD", iden, "", "");
			if (id == 34) { //{
				printWord(getsym());
			}
			/**/
			compoundStatement();
			/**/
			id = watch();
			if (id == 35) {//}
				printWord(getsym());
			}
			sym_table.outFunction();
			funcRecord[iden] = nameList;
			middleTable.addDefine("FUNCRET", "", "", "");
			middleTable.addDefine("FUNCTAIL", "", "", "");
		}
	}
	if (debug)
		fprintf(fc, "<无返回值函数定义>\n");
}

void mainFunc() {
	middleTable.addDefine("MAINHEAD", "", "", "");
	int id = watch();
	if (id == 7) {//void
		printWord(getsym());
	}
	id = watch();
	if (id == 8) {//main
		printWord(getsym());
	}
	sym_table.inFunction(VOID);
	sym_table.add(FUNCTION, VOID);
	sym_table.setParaNum(0);
	id = watch();
	if (id == 30) {//(
		printWord(getsym());
	}
	else
		printError('l');
	id = watch();
	if (id == 31) {//)
		printWord(getsym());
	}
	id = watch();
	if (id == 34) {//{
		printWord(getsym());
	}
	compoundStatement();
	id = watch();
	if (id == 35) {//}
		printWord(getsym());
	}
	if (debug)
		fprintf(fc, "<主函数>\n");
	sym_table.outFunction();
	sym_table.printTable();
}

void project() {
	int id = watch();
	// 常量说明
	if (id == 4) {  //constant
		constState();
	}
	// 变量说明
	if ((watch() == 5 || watch() == 6) && watchThrice() != 30) {
		varState();
	}
	// 有无返回值函数定义
	id = watch();
	while (id == 5 || id == 6 || id == 7) {
		if (id == 5 || id == 6) {
			returnFunc();
			id = watch();
		}
		else if (id == 7) {
			if (watchTwice() == 8) {
				break;
			}
			else if (watchTwice() == 0) {
				noReturnFunc();
				id = watch();
			}
		}
	}
	id = watch();
	mainFunc();
	middleTable.addDefine("EXIT", "", "", "");
	if (debug)
		fprintf(fc, "<程序>\n");
	middleTable.midFile.close();
	
}