#pragma once

#define condi(x,y) (midCode.op2 == x && midCode.Type == y)

void translate();

void dotData_gen();

void dotText_gen();

void translate();

int dispatchReg(string name);

int searchData(string name, string& globalName, string& type);

void dispatch(string name,string type);

string loadVarToReg(string name, string no, string regName);

void storeRegToVar(string name, string no, string regName);

void sentence(middleCode midCode);

void printSentence(middleCode midCode);

void scanSentence(middleCode midCode);

void calSentence(middleCode midCode);

void loadArrSentence(middleCode midCode);

void storeArrSentence(middleCode midCode);

void callFuncSentence(middleCode midCode);

void pushParaSentence(middleCode midCode);

void conditionSentence(middleCode midCode);

void conditionJumpSentence(middleCode midCode);

void constDefineSentence(middleCode midCode);

void varDefineSentence(middleCode midCode);