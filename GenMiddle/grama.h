//statement type
#define CONDITION 0
#define LOOP 1
#define COLUMN 2
#define SCANF 3
#define PRINTF 4
#define RETURN 5
#define NORETURN 6
#define ASSIGN 7
#define FUNCRETURN 8
#define EMPTY 9
extern middleCodeTable middleTable;
extern map<string, vector<middleCode>> funcParaRecord;

void constDefine();

void constState();

void varDefine();

void varState();

int statementClassifer();

void valueTable(string funcName);

void returnFuncStatement(string& name);

void noReturnFuncStatement();

int factor(string& name);

int term(string& name);

int expression(string& name);

void condition(string& condi);

void loopStatement();

void conditionStatement();

void scanfStatement();

void printfStatement();

void assignStatement();

void funcReturnStatement();

void statement();

void statementColumn();

void compoundStatement();

void recordFunc();

void recordNoFunc();

void returnFunc();

void noReturnFunc();

void mainFunc();

void project();