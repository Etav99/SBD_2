#pragma once
#include "Record.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

struct Reader {
	string fileName;
	int blockStartIndex;
	int blockEndIndex;
	int bufferSize;
	int lastRecordIndex;
	bool bufferEmpty;
	int *readNo;

	Record* buffer;

	Reader(string fileName, int bufferSize);
	Reader(string fileName, int bufferSize, int* readNo);
	void updateSize();
	bool loadBlock(int blockIndex);
	Record* readRecord(int recordNumber);
	~Reader();
};