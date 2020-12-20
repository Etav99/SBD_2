#include <iostream>
#include "Settings.h"
#include "Tape.h"
#include "Record.h"
#include "Reader.h"
#include "Writer.h"
#include <cstdlib>
#include <ctime>

#define BLOCK_SIZE 50
#define LOWER_RAND 1.0
#define UPPER_RAND 50.0

bool showEachPhase = 1;
bool showDistribution = 0;
int phaseNo = 0;

using namespace std;

void distribute(Tape* input, Tape* tape1, Tape* tape2) {
	tape1->clear();
	tape2->clear();
	Tape* tape[2] = { tape1, tape2 };
	bool activeTape = 0;

	input->startReading();
	if (input->endOfTape)
		return;
	tape1->addRecord(input->currentRecord);
	input->nextRecord();

	while (!(input->endOfTape)) {
		if (input->currentRecord < input->previousRecord)
			activeTape = !activeTape;
		tape[activeTape]->addRecord(input->currentRecord);
		input->nextRecord();
	}

	tape1->finishWriting();
	tape2->finishWriting();


	if (showDistribution) {
		cout << "________________________________________: " << endl;
		cout << "Poczatkowa dystrybucja: " << endl;
		input->printTape();
		tape1->printTape();
		tape2->printTape();
		cout << "________________________________________: " << endl << endl;
	}

}


bool merge(Tape* tape1, Tape* tape2, Tape* tape3) {
	tape1->startReading();
	tape2->startReading();

	if (tape2->endOfTape)
		return true;
	else
		tape3->clear();



	while (true) {
		//Sprawdź czy taśmy nie są puste
		if (!(tape1->endOfTape) && !(tape2->endOfTape)) {
			// Jeśli koniec serii na taśmie 1 to przepisz resztę serii z taśmy 2 na 3
			if (tape1->endOfRun) {
				while (!(tape2->endOfTape) && !(tape2->endOfRun)) {
					tape3->addRecord(tape2->currentRecord);
					tape2->nextRecord();
				}
				tape1->newRun();
				tape2->newRun();
			}
			// Jeśli koniec serii na taśmie 2 to przepisz resztę serii z taśmy 1 na 3
			else if (tape2->endOfRun) {
				while (!(tape1->endOfTape) && !(tape1->endOfRun)) {
					tape3->addRecord(tape1->currentRecord);
					tape1->nextRecord();
				}
				tape1->newRun();
				tape2->newRun();
			}
			// W przeciwnym wypadku porównaj rekordy i przepisz mniejszy
			else {
				if (tape1->currentRecord < tape2->currentRecord) {
					tape3->addRecord(tape1->currentRecord);
					tape1->nextRecord();
				}
				else {
					tape3->addRecord(tape2->currentRecord);
					tape2->nextRecord();
				}
			}
		}
		else if (tape1->endOfTape) {
			while (!(tape2->endOfTape)) {
				tape3->addRecord(tape2->currentRecord);
				tape2->nextRecord();
			}
			break;
		}
		else if (tape2->endOfTape) {
			while (!(tape1->endOfTape)) {
				tape3->addRecord(tape1->currentRecord);
				tape1->nextRecord();
			}

			break;
		}
	}
	tape3->finishWriting();


	if (showEachPhase) {
		cout << "________________________________________: " << endl;
		cout << "Faza nr " << phaseNo << ":" << endl;
		tape1->printTape();
		tape2->printTape();
		tape3->printTape();
		cout << "________________________________________: " << endl << endl;
	}

	return false;

}


void generate(Tape* inputTape, int recordsNumber) {
	remove(inputTape->filename.c_str());
	srand(time(NULL));
	int i = 0;
	Record record;
	while (i < recordsNumber) {
		do {
			record.x = rand() % 200 + 1;
			record.a = LOWER_RAND + (float(rand()) / (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
			record.b = LOWER_RAND + (float(rand()) / (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
			record.c = LOWER_RAND + (float(rand()) / (RAND_MAX / (UPPER_RAND - LOWER_RAND)));
		} while (!Record::deltaCheck(record));
		inputTape->addRecord(record);
		i++;
	}
	inputTape->finishWriting();
}

void sort(Tape* inputTape) {
	phaseNo = 0;
	cout << "Plik wejsciowy:\n";
	inputTape->printTape();
	remove("tape1");
	remove("tape2");
	remove("tape3");
	Tape tape1("tape1", BLOCK_SIZE);
	Tape tape2("tape2", BLOCK_SIZE);
	Tape tape3("tape3", BLOCK_SIZE);
	bool sorted = 0;
	distribute(inputTape, &tape1, &tape2);
	sorted = merge(&tape1, &tape2, &tape3);
	phaseNo++;
	while (!sorted) {
		distribute(&tape3, &tape1, &tape2);
		sorted = merge(&tape1, &tape2, &tape3);
		phaseNo++;
	}

	cout << "Plik posortoway:\n";
	tape3.printTape();

	cout << "\n\nLiczba faz: " << phaseNo << endl;

}




int main()
{
	remove("generated");
	remove("keyboardInput");
	char in;
	int recordsNumber;
	Record inputRecord;
	Tape* inputTape;
	string filename;
	do {
		system("cls");
		cout << "Maciej Zakrzewski\nNr albumu: 175573\n\nSortowanie naturalne w schemacie 2+1\n\nRekordy pliku: wspolczynniki wielomianow kwadratowych\nSortowanie wzgledem sumy pierwiastkow wielomianow\n\n";
		cout << "Menu:\n1. Sortuj wygenerowane dane\n2.Sortuj dane wprowadzone z klawiatury\n3.Sortuj dane z pliku testowego\n\nWybor: ";
		cin >> in;
		switch (in) {
		case '1':
			do {
				cout << "\nPodaj liczbe rekordow: ";
				cin >> recordsNumber;
			} while (cin.fail());
			inputTape = new Tape("generated", BLOCK_SIZE);
			generate(inputTape, recordsNumber);
			inputTape->printTape();
			sort(inputTape);
			system("pause");
			break;
		case '2':
			do {
				cout << "\nPodaj liczbe rekordow: ";
				cin >> recordsNumber;
			} while (cin.fail());
			inputTape = new Tape("keyboardInput", BLOCK_SIZE);
			cout << "Wprowadzaj po trzy wspolczynniki oddzielone spacja";
			for (int i = 1; i <= recordsNumber;) {
				cout << "Rekord " << i << ":";
				cin >> inputRecord.x;
				if (cin.fail()) {
					cout << "Nieprawidlowa wartosc, sprobuj ponownie.";
					continue;
				}
				i++;
				inputTape->addRecord(inputRecord);

				// cin >> b;
				// if (cin.fail()) {
				//     cout << "Nieprawidlowa wartosc, sprobuj ponownie.";
				//     continue;
				// }
				//  cin >> c;
				// if (cin.fail()) {
				//     cout << "Nieprawidlowa wartosc, sprobuj ponownie.";
				//     continue;
				// }
			}
			inputTape->finishWriting();
			sort(inputTape);
			system("pause");
			break;
		case '3':
			do {
				cout << "\nPodaj nazwe pliku testowego: ";
				cin >> filename;
			} while (cin.fail());
			inputTape = new Tape(filename, BLOCK_SIZE);
			sort(inputTape);
			break;
		case 'q':
			break;

		}
	} while (in != 'q');

	remove("test");
	remove("tape1");
	remove("tape2");
	remove("tape3");
	Writer* writer = new Writer("test", 50);
	Reader* reader = new Reader("test", 50);
	double x = 0.0;
	int war[8] = { 44, 55, 12, 42, 94, 18, 6, 67 };
	Record record;
	for (int i = 0; i < 8; i++)
	{
		record.x = war[i];
		writer->writeRecord(record);
	}
	writer->writeBlock();

	Tape input("test", BLOCK_SIZE);
	Tape tape1("tape1", BLOCK_SIZE);
	Tape tape2("tape2", BLOCK_SIZE);
	Tape tape3("tape3", BLOCK_SIZE);
	phaseNo++;
	distribute(&input, &tape1, &tape2);
	merge(&tape1, &tape2, &tape3);
	phaseNo++;
	distribute(&tape3, &tape1, &tape2);
	merge(&tape1, &tape2, &tape3);



}
