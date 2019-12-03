#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<iomanip>
using namespace std;

#define isNumStr(x) ((x[0] >= '0' && x[0] <= '9') || ((x[0] == '+' || x[0] == '-')&&(x[1] >= '0' && x[1] <= '9')))

void clearToken();

void getChar();

void addToken();

void finishToken();

int isunderline(char c);

void back();

int matcher();

void newLine();

bool isChar(char ch);

bool isStr(char ch);

int getsym();

int watch();

int watchTwice();

int watchThrice();

void printWord(int id);