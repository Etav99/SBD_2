#include "Buffer.h"
#include "Index.h"
#include "Record.h"
#include <iostream>
#define PAGE_SIZE 6
#define OVERFLOW_SIZE 0.2
#define ALPHA 0.5
#define LOWER_RAND -25.0
#define UPPER_RAND 25.0

using namespace std;

void generate(Record* record) {
	record->a = LOWER_RAND + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
	record->b = LOWER_RAND + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
	record->c = LOWER_RAND + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
}

void beforeOperation(Buffer& buffer, int& readNo, int& writeNo) {
	cout << "\n#############################################################################################\n";
	cout << "#                                       PRZED OPERACJA                                     #";
	cout << "\n#############################################################################################\n";
	buffer.display();
	cout << "\n\n#############################################################################################\n";
	cout << "#############################################################################################\n";
	readNo = 0;
	writeNo = 0;
}

void afterOperation(Buffer& buffer, int& readNo, int& writeNo) {
	int reads = readNo;
	int writes = writeNo;
	cout << "\n#############################################################################################\n";
	cout << "#                                       PO OPERACJI                                        #";
	cout << "\n#############################################################################################\n";
	buffer.display();
	cout << "\n\nLiczba odczytow: " << reads << endl;
	cout << "Liczba zapisow: " << writes << endl;
	cout << "#############################################################################################\n";
	cout << "#############################################################################################\n";
	system("PAUSE");
}

void afterFindOperation(int& readNo, int& writeNo) {
	cout << "\n\nLiczba odczytow: " << readNo << endl;
	cout << "Liczba zapisow: " << writeNo << endl << endl;
	system("PAUSE");
}

int main(int argc, char* argv[])
{
	cout << sizeof(Record) << " " << sizeof(Index);
	int readNo = 0, writeNo = 0;
	unsigned int page_size = PAGE_SIZE;
	float alpha = ALPHA, overflowsize = OVERFLOW_SIZE;
	bool testing = false;
	int x;
	char in;
	Record inputRecord;
	string filename;
	string testFilename;

	if (argc >= 2 && strcmp(argv[1], "-t")) {
		testing = true;
	}


	if (testing) {
		Buffer* testBuffer;
		fstream numbers1("numbers1.txt", fstream::in);
		fstream numbers2("numbers2.txt", fstream::in);
		int p = 10;


		for (int N = 200; N <= 1000; N += 200) {
			for (float A = 0.1; A <= 1.09; A += 0.1) {
				if (A > 1)
					A = 1;
				for (float B = 0.1; B <= 1.09; B += 0.1) {
					if (B > 1)
						B = 1;
					testBuffer = new Buffer("test", p, A, B, &readNo, &writeNo);
					testBuffer->createNewFile();
					testBuffer->createIndex();
					int i;
					//dodawanie
					readNo = writeNo = 0;
					numbers1.seekg(0, numbers1.beg);
					numbers2.seekg(0, numbers2.beg);
					for (i = 0; i < N; i++) {
						numbers1 >> inputRecord.key;
						generate(&inputRecord);
						testBuffer->addRecord(inputRecord);
					}
					cout << "A " << i << " " << p << " " << A << " " << B << " " << readNo << " " << writeNo << " " << testBuffer->mainPages + testBuffer->overflowPages << " " << testBuffer->indexPages << endl;


					//aktualizacja
					readNo = writeNo = 0;
					numbers1.seekg(0, numbers1.beg);
					numbers2.seekg(0, numbers2.beg);
					int key;
					for (i = 0; i < N; i++) {
						numbers1 >> key;
						numbers2 >> inputRecord.key;
						generate(&inputRecord);
						testBuffer->updateRecord(key, inputRecord);
					}
					cout << "U " << i << " " << p << " " << A << " " << B << " " << readNo << " " << writeNo << " " << testBuffer->mainPages + testBuffer->overflowPages << " " << testBuffer->indexPages << endl;

					//usuwanie
					readNo = writeNo = 0;
					numbers1.seekg(0, numbers1.beg);
					numbers2.seekg(0, numbers2.beg);
					for (i = 0; i < N; i++) {
						numbers2 >> key;
						testBuffer->deleteRecord(key);
					}
					cout << "D " << i << " " << p << " " << A << " " << B << " " << readNo << " " << writeNo << " " << testBuffer->mainPages + testBuffer->overflowPages << " " << testBuffer->indexPages << endl;
					delete testBuffer;
				}
			}
		}
		return 0;
	}

	Buffer buffer("files", page_size, alpha, overflowsize, &readNo, &writeNo);
	buffer.createNewFile();
	buffer.createIndex();
	int reads, writes;
	cout << fixed << std::setprecision(2) << setfill(' ');
	do {
		system("cls");
		cout << "Maciej Zakrzewski\nNr albumu: 175573\n\nPlik sekwencyjno-indeksowy\n\nRekordy pliku: wspolczynniki wielomianow kwadratowych\nSortowanie wedlug klucza\n\n";
		cout << "Wspolczynnik b = " << page_size << endl << "Wspolczynnik alfa = " << alpha << endl << "Rozmiar overflow = " << overflowsize << endl << endl;
		cout << "Menu:\na. Dodaj rekord\nd. Usun rekord\nu. Zaktualizuj rekord\np. Wyswietl plik\ni. Wyswietl indeks\nr. Reorganizuj\nf. Znajdz rekord\nt. Wykonaj dzialania z pliku testowego\nq. Wyjscie\n\nWybor: ";
		cin >> in;
		cout << "\n";
		switch (in) {
		case 'a':
			cout << "Wprowadz klucz rekordu(wartosci zostana wygenerowane): ";
			while (true) {
				cin >> inputRecord.key;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				generate(&inputRecord);
				beforeOperation(buffer, readNo, writeNo);
				if (buffer.addRecord(inputRecord) == 0) {
					cout << "Rekord o podanym kluczu już istnieje\n";
					system("PAUSE");
				}
				afterOperation(buffer, readNo, writeNo);
				break;
			}
			break;
		case 'p':
			readNo = 0;
			writeNo = 0;
			buffer.display();
			afterFindOperation(readNo, writeNo);
			break;
		case 'd':
			cout << "Wprowadz klucz rekordu do usuniecia: \n\n";
			while (true) {
				cin >> inputRecord.key;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				beforeOperation(buffer, readNo, writeNo);
				if (buffer.deleteRecord(inputRecord.key) == 0) {
					cout << "Brak rekordu o podanym kluczu\n";
					system("PAUSE");
				}
				afterOperation(buffer, readNo, writeNo);
				break;
			}
			break;
		case 'r':
			beforeOperation(buffer, readNo, writeNo);
			buffer.reorganize();
			afterOperation(buffer, readNo, writeNo);
			break;
		case 'u':
			cout << "Wprowadz klucz rekordu do aktualizacji: ";
			while (true) {
				cin >> x;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				break;
			}
			cout << "Wprowadz nowy klucz rekordu(nowe wartosci zostana wygenerowane): ";
			while (true) {
				cin >> inputRecord.key;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				generate(&inputRecord);
				beforeOperation(buffer, readNo, writeNo);
				if (buffer.updateRecord(x, inputRecord) == 0) {
					cout << "Brak rekordu o podanym kluczu\n";
					system("PAUSE");
				}
				afterOperation(buffer, readNo, writeNo);
				break;
			}
			break;
		case 'f':
			cout << "Wprowadz klucz rekordu: ";
			while (true) {
				cin >> inputRecord.key;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				readNo = writeNo = 0;
				buffer.findRecord(inputRecord.key);
				cout << "\n";
				afterFindOperation(readNo, writeNo);
				break;
			}
		case 'i':
			readNo = writeNo = 0;
			buffer.displayIndex();
			afterFindOperation(readNo, writeNo);
			cout << "\n\n";
			break;
		case 'q':
			break;
		case 't':
			cout << "Wprowadz nazwe pliku testowego: ";
			while (true) {
				cin >> testFilename;
				if (cin.fail()) {
					cout << "\n\nNieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				break;
			}

			fstream test(testFilename + ".txt", fstream::in);
			if (!test.is_open()) {
				cout << "Błąd otwierania pliku\n\n";
				system("PAUSE");
				break;
			}

			beforeOperation(buffer, readNo, writeNo);

			readNo = writeNo = 0;
			while (!test.eof()) {
				test >> in;
				switch (in)
				{
				case 'a':
					test >> inputRecord.key;
					generate(&inputRecord);
					buffer.addRecord(inputRecord);
					break;
				case 'd':
					test >> inputRecord.key;
					buffer.deleteRecord(inputRecord.key);
					break;
				case 'u':
					test >> x;
					test >> inputRecord.key;
					generate(&inputRecord);
					buffer.updateRecord(x, inputRecord);
					break;
				case 'r':
					buffer.reorganize();
					break;
				default:
					break;
				}
			}
			afterOperation(buffer, readNo, writeNo);
			break;

		}
	} while (in != 'q');
}
