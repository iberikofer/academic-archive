#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
using namespace std;

struct ModeSettings
{
	string deviceName = "COM1";
	int speed = 9600;
	char parity = 'N';
	int dataBits = 8;
	int stopBits = 1;
};

ModeSettings chosenModeSettings;
int optionChosen = 0;
int commandChosen = 0;
int MODEmenuChoice = 0;
bool inMenu = true;
string fileToErase;
string fileToCopy;
string fileToCopyDest;

int getSafeInt()
{
	int value;
	while (!(cin >> value))
	{
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cerr << "Error! Please enter a single NUMBER: ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return value;
}

void menuChoice()
{
	cout << "Hello world! Please choose a command...(enter a number)" << endl
			 << "1 - MODE" << endl
			 << "2 - ERA" << endl
			 << "3 - PIP" << endl
			 << "4 - Type myself" << endl
			 << "5 - Exit" << endl
			 << endl;
	commandChosen = getSafeInt();
}

void printModeSettings()
{
	cout << endl
			 << "---Chosen ModeSettings---" << endl
			 << "Chosen deviceName: " << chosenModeSettings.deviceName << endl
			 << "Chosen speed: " << chosenModeSettings.speed << endl
			 << "Chosen parity: " << chosenModeSettings.parity << endl
			 << "Chosen stopBits: " << chosenModeSettings.stopBits << endl
			 << endl;
}

int main()
{
	int exitConfirmed = 0;
	system("color 0A");

	do
	{
		menuChoice();

		switch (commandChosen)
		{
		case (1):
{
    cout << "Status for device " << chosenModeSettings.deviceName << ":" << endl;
    cout << "-------------------------------------------" << endl;
    cout << "    Baud:            " << chosenModeSettings.speed << endl;
    cout << "    Parity:          " << chosenModeSettings.parity << endl;
    cout << "    Data Bits:       " << chosenModeSettings.stopBits + 7;
    cout << "\n    Stop Bits:       " << chosenModeSettings.stopBits << endl;
    cout << "    Timeout:         OFF" << endl << endl;

		cout << "Current Console Status (CON):" << endl;
		system("mode CON"); 
    cout << endl;
}
break;
		case (2):
		{
			cout << "You chose ERA." << endl
					 << "Which file do you want to Delete?" << endl;
			cin >> fileToErase;
			if (!remove(fileToErase.c_str()))
				cout << "File removed successfully!" << endl;
			else
				cerr << "Error deleting the file!" << endl;
		}
		break;
		case (3):
		{
			cout << "You chose PIP." << endl;

			cout << "Enter source file path: ";
			cin >> fileToCopy;

			ifstream checkFile(fileToCopy);
			if (!checkFile.is_open())
			{
				cerr << "Error: Source file does not exist at " << fileToCopy << endl
						 << endl;
				break;
			}
			checkFile.close();

			cout << "Enter destination file path: ";
			cin >> fileToCopyDest;

			ifstream source(fileToCopy, ios::binary);
			ofstream dest(fileToCopyDest, ios::binary);

			if (source.is_open() && dest.is_open())
			{
				dest << source.rdbuf();
				cout << "File copied successfully to " << fileToCopyDest << "!" << endl
						 << endl;
			}
			else
			{
				cerr << "Error: Could not create destination file at " << fileToCopyDest << endl
						 << endl;
			}
		}
		break;
		case (4):
		{
			cout << "Enter your command: " << endl;
			string fullCommand;
			getline(cin, fullCommand);

			istringstream iss(fullCommand);
			string commandKeyword;
			iss >> commandKeyword;
			for (auto &c : commandKeyword)
				c = toupper(c);

			if (commandKeyword == "ERA")
			{
				if (iss >> fileToErase)
				{
					if (!remove(fileToErase.c_str()))
						cout << "Successfully removed!" << endl;
					else
						cerr << "Error removing the file!" << endl;
				}
				else
				{
					cerr << "Usage: ERA <filename>" << endl;
				}
			}
			else if (commandKeyword == "PIP")
			{
				if (iss >> fileToCopy >> fileToCopyDest)
				{
					ifstream source(fileToCopy, ios::binary);
					ofstream dest(fileToCopyDest, ios::binary);

					if (source.is_open() && dest.is_open())
					{
						dest << source.rdbuf();
						cout << "File copied successfully to " << fileToCopyDest << "!" << endl;
					}
					else
					{
						cerr << "Error: Could not perform PIP (check file paths)." << endl;
					}
				}
				else
				{
					cerr << "Usage: PIP <source> <destination>" << endl;
				}
			}
			else if (commandKeyword == "MODE")
			{
				string target;
				if (!(iss >> target))
				{
					system("mode");
				}
				else
				{
					for (auto &c : target)
						c = toupper(c);

					if (target.find("CON") != string::npos)
					{
						system(fullCommand.c_str());
						cout << "System display/console settings updated." << endl;
					}
					else
					{
						chosenModeSettings.deviceName = target;
						string param;
						while (iss >> param)
						{
							for (auto &c : param)
								c = toupper(c);
							if (param.find("BAUD=") == 0)
								chosenModeSettings.speed = stoi(param.substr(5));
							if (param.find("PARITY=") == 0)
								chosenModeSettings.parity = param[7];
							if (param.find("STOP=") == 0)
								chosenModeSettings.stopBits = stoi(param.substr(5));
						}
						cout << "Local settings for " << target << " updated." << endl;
						printModeSettings();
					}
				}
			}
			else
			{
				cerr << "Unknown command! Use ERA, PIP or MODE." << endl;
			}
		}
		break;
		case (5):
			cout << "Goodbye, have a great one!" << endl;
			break;
		}
	} while (commandChosen != 5);
	return 0;
}