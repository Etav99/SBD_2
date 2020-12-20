#pragma once
#include "Record.h"
#include <fstream>
#include <iostream>
#include <string>
#include "Index.h"
#include <iomanip>

using namespace std;

struct Buffer
{
	string fileName;
	fstream *file, *indexFile;
	int bufferSize, indexBufferSize;
	int mainAreaSize, overflowAreaSize, overflowRecords, mainPages, overflowPages;
	int loadedPage, loadedIndexPage;
	int indexPages;
	bool bufferEmpty, indexBufferEmpty;
	int* readNo, * writeNo;
	float alpha;
	float overflowRatio;

	Record* buffer;
	Index* indexBuffer;

	Buffer(string fileName, int bufferSize, float alpha, float overflowRatio);
	Buffer(string fileName, int bufferSize, float alpha, float overflowRatio, int* readNo, int* writeNo);
	void createNewFile();
	bool addRecord(Record record);
	void createIndex();
	bool writePage();
	void reorganize();
	bool writeIndexPage();
	int findIndex(int key);
	void loadPage(int pagePointer);
	void loadIndexPage(int pagePointer);
	void clearBuffer(int start = 0);
	void clearIndexBuffer(int start = 0);
	void display();
	void displayIndex();
	bool updateRecord(int key, Record record);
	void findRecord(int key);
	bool deleteRecord(int key);
	~Buffer();
};

