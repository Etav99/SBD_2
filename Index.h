#pragma once
struct Index
{
	int key;
	int page;

	Index();
	Index(int key, int page);
	bool isEmpty();
};


