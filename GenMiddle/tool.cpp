#include"includeH.h"
void printSet(set<string> sset) {
	cout << "printSet:" << endl;
	for (set<string>::iterator it = sset.begin(); it != sset.end(); it++) {
		cout << *it << endl;
	}
}

void printMap(map<string, int> mmap) {
	cout << "printMap:" << endl;
	for (map<string, int>::iterator it = mmap.begin(); it != mmap.end(); it++) {
		cout << it->first+":"+to_string(it->second) << endl;
	}
}