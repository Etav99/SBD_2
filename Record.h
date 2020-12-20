#pragma once
#include <iostream>
using namespace std;

struct Record {
	int key;
	double a, b, c;
	int overflowPointer;
	bool deleted;

	Record();
	Record(int key, double a, double b, double c);
	bool isEmpty();
	bool isGuard();

};