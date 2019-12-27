#include "includeH.h"
FILE* fp, *fl, *fc, * fe;
char str[100000];
bool isError = false;

int main() {
	char c;
	int len = 0;

	fp = fopen("testfile.txt", "r");
	while ((c = fgetc(fp)) != EOF) {
		str[len] = c;
		len++;
	}
	fl = fopen("lexi.txt", "w");
	fc = fopen("gramma.txt", "w");
	fe = fopen("error.txt", "w");

	// grama.c
	project();
	if (!isError) {
		diffFuncAndExit();
		optimize();
		translate();

	}
	//fseek(fc, -1, SEEK_END);
	//fputc('\0', fc);

	
	fclose(fp);
	fclose(fc);
	fclose(fe);
	fclose(fl);
	return 0;
}