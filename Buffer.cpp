#include "Buffer.h"


Buffer::Buffer(string fileName, int bufferSize, float alpha, float overflowRatio) {
	this->fileName = fileName;
	this->bufferEmpty = true;
	this->bufferSize = bufferSize;
	this->indexBufferSize = bufferSize * (sizeof(Record) / sizeof(Index));
	this->buffer = new Record[bufferSize];
	this->indexBuffer = new Index[indexBufferSize];
	remove(fileName.c_str());
	remove((fileName + "Index").c_str());
	file = new fstream(fileName, fstream::out | fstream::binary | fstream::app);
	indexFile = new fstream(fileName + "Index", fstream::out | fstream::binary | fstream::app);
	file->close();
	indexFile->close();
	file->open(fileName, fstream::out | fstream::in | fstream::binary);
	indexFile->open(fileName + "Index", fstream::out | fstream::in | fstream::binary);
	this->readNo = this->writeNo = NULL;
	this->alpha = alpha;
	this->overflowRatio = overflowRatio;
}

Buffer::Buffer(string fileName, int bufferSize, float alpha, float overflowRatio, int* readNo, int* writeNo) : Buffer::Buffer(fileName, bufferSize, alpha, overflowRatio) {
	this->readNo = readNo;
	this->writeNo = writeNo;
}

bool Buffer::addRecord(Record record) {
	loadPage(findIndex(record.key));

	auto insert = [&](int pos)
	{
		buffer[pos] = record;
		writePage();
		return 1;
	};

	for (int i = 0; i < bufferSize - 1; i++) {

		if ((buffer[i].key == record.key && !buffer[i].deleted) || (buffer[i + 1].key == record.key && !buffer[i + 1].deleted))
			return 0;

		if (buffer[i].key == record.key && buffer[i].deleted)
			return insert(i);

		if (buffer[i + 1].key == record.key && buffer[i + 1].deleted)
			return insert(i + 1);

		if ((buffer[i].key < record.key && buffer[i + 1].key > record.key) || buffer[i + 1].isEmpty() || i + 1 == bufferSize - 1)
		{
			if (buffer[bufferSize - 1].isEmpty()) {

				for (int j = bufferSize - 2; j > i; j--)
					buffer[j + 1] = buffer[j];
				return insert(i + 1);
			}
			else {
				if (i + 1 == bufferSize - 1 && buffer[i + 1].key < record.key)
					i = i + 1;
				while (buffer[i].overflowPointer != -1) {
					int previousPage = loadedPage, previousPosition = i;
					int pointer = buffer[i].overflowPointer;
					loadPage(mainPages + buffer[i].overflowPointer / bufferSize);
					i = pointer % bufferSize;
					if (buffer[i].key == record.key && buffer[i].deleted) {
						record.overflowPointer = buffer[i].overflowPointer;
						return insert(i);
					}

					if (buffer[i].key > record.key) {
						loadPage(mainPages + (overflowRecords) / bufferSize);
						record.overflowPointer = pointer;
						buffer[(overflowRecords) % bufferSize] = record;
						writePage();
						loadPage(previousPage);
						buffer[previousPosition].overflowPointer = overflowRecords++;
						writePage();
						if (overflowRecords == overflowAreaSize) {
							reorganize();
						}
						return 1;
					}
				}
				buffer[i].overflowPointer = overflowRecords++;
				writePage();
				loadPage(mainPages + (overflowRecords - 1) / bufferSize);
				buffer[(overflowRecords - 1) % bufferSize] = record;
				writePage();
				if (overflowRecords == overflowAreaSize)
					reorganize();
				return 1;
			}

		}
	}

	return 0;
}

void Buffer::reorganize()
{
	fstream newFile(fileName + "Copy", fstream::out | fstream::binary | fstream::app);
	Record* tempBuffer = new Record[bufferSize];
	int recordsInBuffer = 1, pagesWritten = 0;
	Record empty;

	auto write = [&](bool ifFull)
	{
		if (recordsInBuffer >= bufferSize * alpha || (!ifFull && recordsInBuffer > 0)) {
			for (int i = recordsInBuffer; i < bufferSize; i++)
				tempBuffer[i] = empty;
			newFile.write((char*)tempBuffer, sizeof(Record) * bufferSize);
			if (writeNo != NULL)
				(*writeNo)++;
			pagesWritten++;
			recordsInBuffer = 0;
		}
	};

	tempBuffer[0] = Record(-1, 0, 0, 0);
	tempBuffer[0].deleted = 1;

	for (int i = 0; i < mainPages; i++) {
		loadPage(i);
		for (int j = 0; j < bufferSize; j++)
		{
			if (buffer[j].isEmpty())
				break;

			if (!buffer[j].deleted) {
				tempBuffer[recordsInBuffer] = buffer[j];
				tempBuffer[recordsInBuffer++].overflowPointer = -1;
				write(true);
			}

			int k = j;
			while (buffer[k].overflowPointer != -1) {
				int pointer = buffer[k].overflowPointer;
				loadPage(mainPages + pointer / bufferSize);
				k = pointer % bufferSize;
				if (!buffer[k].deleted) {
					tempBuffer[recordsInBuffer] = buffer[k];
					tempBuffer[recordsInBuffer++].overflowPointer = -1;
					write(true);
				}

			}
			loadPage(i);
		}
	}
	write(false);
	mainPages = pagesWritten;
	mainAreaSize = mainPages * bufferSize;
	overflowAreaSize = mainAreaSize * overflowRatio;
	if (overflowAreaSize == 0)
		overflowAreaSize++;

	int newOverflowPages = overflowAreaSize;
	if (newOverflowPages % bufferSize == 0)
		newOverflowPages /= bufferSize;
	else
		newOverflowPages = newOverflowPages / bufferSize + 1;

	for (int i = 0; i < bufferSize; i++)
		tempBuffer[i] = empty;
	for (int i = 0; i < newOverflowPages; i++) {
		newFile.write((char*)tempBuffer, sizeof(Record) * bufferSize);
	}
	overflowPages = newOverflowPages;
	overflowRecords = 0;

	newFile.close();
	file->close();
	remove(fileName.c_str());
	remove((fileName + "Index").c_str());
	rename((fileName + "Copy").c_str(), fileName.c_str());
	file->open(fileName, fstream::out | fstream::in | fstream::binary);
	createIndex();
	delete[] tempBuffer;
}

void Buffer::createNewFile()
{
	file->close();
	remove((fileName).c_str());
	file->open(fileName, fstream::app | fstream::binary);
	clearBuffer();
	buffer[0] = Record(-1, 0, 0, 0);
	buffer[0].deleted = true;
	file->write((char*)buffer, sizeof(Record) * bufferSize);
	buffer[0] = Record();
	file->write((char*)buffer, sizeof(Record) * bufferSize);
	loadedPage = 1;
	file->close();
	file->open(fileName, fstream::out | fstream::in | fstream::binary);
	mainAreaSize = bufferSize;
	mainPages = 1;
	overflowPages = 1;
	overflowAreaSize = mainAreaSize * overflowRatio;
	if (overflowAreaSize == 0)
		overflowAreaSize++;
	overflowRecords = 0;
}

void Buffer::createIndex() {
	indexPages = 0;
	indexFile->close();
	remove((fileName + "Index").c_str());
	clearIndexBuffer();
	indexFile->open(fileName + "Index", fstream::out | fstream::binary | fstream::app);
	for (int i = 0, j = 0; i < mainPages; i++, j++) {
		loadPage(i);
		indexBuffer[j] = Index(buffer[0].key, i);
		if (j == indexBufferSize - 1) {
			loadedPage = indexPages;
			writeIndexPage();
			clearIndexBuffer();
			loadedIndexPage = -1;
			indexPages++;

			j = -1;
		}
	}
	if (indexBuffer[0].key != -2) {
		writeIndexPage();
		loadedIndexPage = indexPages;
		indexPages++;

	}
	indexFile->close();
	indexFile->open(fileName + "Index", fstream::in | fstream::app);
}




bool Buffer::writePage() {
	if (loadedPage < 0)
		return false;
	file->seekg(loadedPage * bufferSize * sizeof(Record), ios::beg);
	if (file->write((char*)buffer, sizeof(Record) * bufferSize)) {
		if (writeNo != NULL)
			(*writeNo)++;
		return true;
	}
	return false;
}

bool Buffer::writeIndexPage() {
	if (indexFile->write((char*)indexBuffer, sizeof(Index) * indexBufferSize)) {
		if (writeNo != NULL)
			(*writeNo)++;
		return true;
	}
	return false;
}

int Buffer::findIndex(int key)
{
	for (int i = 0; i < indexPages; i++) {
		loadIndexPage(i);
		int prevPageLastIndex;
		for (int j = 0; j < indexBufferSize; j++) {
			if (indexBuffer[j].key > key || indexBuffer[j].isEmpty()) {
				if (j == 0)
					return prevPageLastIndex;
				return indexBuffer[j - 1].page;
			}
		}
		prevPageLastIndex = indexBuffer[indexBufferSize - 1].page;
	}
	return -1;
}

void Buffer::loadPage(int pagePointer) {
	if (pagePointer < 0) {
		loadedPage = -1;
		return;
	}
	if (loadedPage == pagePointer)
		return;
	file->seekg(pagePointer * bufferSize * sizeof(Record), ios::beg);
	file->read((char*)buffer, sizeof(Record) * bufferSize);
	this->bufferEmpty = false;
	if (readNo != NULL)
		(*readNo)++;
	loadedPage = pagePointer;
}

void Buffer::loadIndexPage(int pagePointer) {
	if (pagePointer < 0) {
		loadedIndexPage = -1;
		return;
	}
	if (loadedIndexPage == pagePointer)
		return;
	indexFile->seekg(pagePointer * indexBufferSize * sizeof(Index), ios::beg);
	indexFile->read((char*)indexBuffer, sizeof(Index) * indexBufferSize);
	this->indexBufferEmpty = false;
	if (readNo != NULL)
		(*readNo)++;
	loadedIndexPage = pagePointer;
}


void Buffer::clearBuffer(int start) {
	loadedPage = -1;
	Record empty;
	for (int i = start; i < bufferSize; i++) {
		buffer[i] = empty;
	}
}

void Buffer::clearIndexBuffer(int start) {
	loadedIndexPage = -1;
	Index empty;
	for (int i = start; i < bufferSize; i++) {
		indexBuffer[i] = empty;
	}
}

void Buffer::display()
{
	for (int i = 0; i < mainPages; i++) {
		loadPage(i);
		cout << "_____________________________________________________________________________________________\n";
		cout << "Strona " << right << setw(3) << i << ":  \n\n";
		int j;
		for (j = 0; j < bufferSize; j++) {
			if (buffer[j].isEmpty())
				break;
			if (buffer[j].deleted)
				cout << "#";
			//cout << "[" << buffer[j].key << "] (" << buffer[j].a << ", " << buffer[j].b << ", " << buffer[j].c <<")\t";
			cout << "[" << buffer[j].key << "] ";

			int x = j;
			while (buffer[x].overflowPointer != -1) {
				int pointer = buffer[x].overflowPointer;
				loadPage(mainPages + buffer[x].overflowPointer / bufferSize);
				x = pointer % bufferSize;
				if (buffer[x].deleted)
					cout << "#";
				cout << "[" << buffer[x].key << "]+ ";
				//cout << "[" << buffer[x].key << "]+ (" << buffer[x].a << ", " << buffer[x].b << ", " << buffer[x].c << ")\t";
			}
			loadPage(i);
		}
		cout << "\n\nWolne miejsca: " << bufferSize - j;
		cout << "\n_____________________________________________________________________________________________\n";
	}
	cout << "Wolne miejsca w overflow: " << overflowAreaSize - overflowRecords;
}

void Buffer::displayIndex()
{
	//cout << "\nKlucz\t\tStrona\n";
	cout << left << setw(16) << setfill(' ') << "Klucz";
	cout << left << setw(16) << setfill(' ') << "Strona" << endl << endl;

	for (int i = 0; i < indexPages; i++) {
		loadIndexPage(i);
		for (int j = 0; j < indexBufferSize; j++) {
			if (indexBuffer[j].isEmpty())
				return;
			cout << left << setw(16) << setfill(' ') << indexBuffer[j].key;
			cout << left << setw(16) << setfill(' ') << indexBuffer[j].page << endl;
			//cout << "\n" << indexBuffer[j].key << "\t\t" << indexBuffer[j].page;
		}
	}
}

bool Buffer::deleteRecord(int key)
{
	loadPage(findIndex(key));
	int k;
	for (int i = 0; i < bufferSize; i++) {
		k = i;
		if (buffer[i].key == key && !buffer[i].deleted) {
			buffer[i].deleted = true;
			writePage();
			return 1;
		}

		while (buffer[k].overflowPointer != -1) {
			int pointer = buffer[k].overflowPointer;
			loadPage(mainPages + pointer / bufferSize);
			k = pointer % bufferSize;
			if (buffer[k].key == key && !buffer[k].deleted)
			{
				buffer[k].deleted = true;
				writePage();
				return 1;
			}
		}
	}
	return 0;
}

bool Buffer::updateRecord(int key, Record record) {
	loadPage(findIndex(key));
	if (key == record.key) {
		for (int i = 0; i < bufferSize; i++) {

			if (buffer[i].key == key) {
				if (buffer[i].deleted)
					return 0;
				buffer[i] = record;
				writePage();
				return 1;
			}

			int k = i;
			while (buffer[k].overflowPointer != -1) {
				int pointer = buffer[k].overflowPointer;
				loadPage(mainPages + pointer / bufferSize);
				k = pointer % bufferSize;
				if (buffer[i].key == key) {
					if (buffer[i].deleted)
						return 0;
					buffer[i] = record;
					writePage();
					return 1;
				}
			}
		}
		return 0;
	}
	else {
		deleteRecord(key);
		addRecord(record);
	}
}

void Buffer::findRecord(int key) {
	loadPage(findIndex(key));
	for (int i = 0; i < bufferSize; i++) {

		if (buffer[i].key == key) {
			if (buffer[i].deleted) {
				cout << "\nNie ma takiego rekordu";
				return;
			}
			cout << "\nZnaleziono rekord:\n[" << buffer[i].key << "] (" << buffer[i].a << ", " << buffer[i].b << ", " << buffer[i].c << ")";
			return;
		}

		int k = i;
		while (buffer[k].overflowPointer != -1) {
			int pointer = buffer[k].overflowPointer;
			loadPage(mainPages + pointer / bufferSize);
			k = pointer % bufferSize;
			if (buffer[i].key == key) {
				if (buffer[i].deleted) {
					cout << "\nNie ma takiego rekordu";
					return;
				}
				cout << "\nZnaleziono rekord:\n[" << buffer[i].key << "] (" << buffer[i].a << ", " << buffer[i].b << ", " << buffer[i].c << ")";
				return;
			}
		}
	}
	cout << "\nNie ma takiego rekordu";
}

Buffer::~Buffer() {
	delete[] buffer;
	delete[] indexBuffer;
	file->close();
	indexFile->close();
}