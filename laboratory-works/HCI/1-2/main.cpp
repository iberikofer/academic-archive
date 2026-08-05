#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <cstdio>

struct ModeSettings
{
	std::string deviceName = "COM1";
	int speed = 9600;
	char parity = 'N';
	int dataBits = 8;
	int stopBits = 1;
};

int getSafeInt()
{
	int value;
	while (!(std::cin >> value))
	{
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cerr << "Error! Please enter a single NUMBER: ";
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return value;
}

int menuChoice()
{
	std::cout << "Hello world! Please choose a command...(enter a number)" << std::endl
			  << "1 - MODE" << std::endl
			  << "2 - ERA" << std::endl
			  << "3 - PIP" << std::endl
			  << "4 - Type myself" << std::endl
			  << "5 - Exit" << std::endl
			  << std::endl;
	return getSafeInt();
}

void printModeSettings(const ModeSettings &settings)
{
	std::cout << std::endl
			  << "---Chosen ModeSettings---" << std::endl
			  << "Chosen deviceName: " << settings.deviceName << std::endl
			  << "Chosen speed: " << settings.speed << std::endl
			  << "Chosen parity: " << settings.parity << std::endl
			  << "Chosen stopBits: " << settings.stopBits << std::endl
			  << std::endl;
}

int main()
{
	ModeSettings chosenModeSettings;
	int commandChosen = 0;
	std::string fileToErase;
	std::string fileToCopy;
	std::string fileToCopyDest;

	system("color 0A");

	do
	{
		commandChosen = menuChoice();

		switch (commandChosen)
		{
		case (1):
		{
			std::cout << "Status for device " << chosenModeSettings.deviceName << ":" << std::endl;
			std::cout << "-------------------------------------------" << std::endl;
			std::cout << "    Baud:            " << chosenModeSettings.speed << std::endl;
			std::cout << "    Parity:          " << chosenModeSettings.parity << std::endl;
			std::cout << "    Data Bits:       " << chosenModeSettings.stopBits + 7;
			std::cout << "\n    Stop Bits:       " << chosenModeSettings.stopBits << std::endl;
			std::cout << "    Timeout:         OFF" << std::endl << std::endl;

			std::cout << "Current Console Status (CON):" << std::endl;
			system("mode CON"); 
			std::cout << std::endl;
		}
		break;
		case (2):
		{
			std::cout << "You chose ERA." << std::endl
					  << "Which file do you want to Delete?" << std::endl;
			std::cin >> fileToErase;
			if (!std::remove(fileToErase.c_str()))
				std::cout << "File removed successfully!" << std::endl;
			else
				std::cerr << "Error deleting the file!" << std::endl;
		}
		break;
		case (3):
		{
			std::cout << "You chose PIP." << std::endl;

			std::cout << "Enter source file path: ";
			std::cin >> fileToCopy;

			std::ifstream checkFile(fileToCopy);
			if (!checkFile.is_open())
			{
				std::cerr << "Error: Source file does not exist at " << fileToCopy << std::endl
						  << std::endl;
				break;
			}
			checkFile.close();

			std::cout << "Enter destination file path: ";
			std::cin >> fileToCopyDest;

			std::ifstream source(fileToCopy, std::ios::binary);
			std::ofstream dest(fileToCopyDest, std::ios::binary);

			if (source.is_open() && dest.is_open())
			{
				dest << source.rdbuf();
				std::cout << "File copied successfully to " << fileToCopyDest << "!" << std::endl
						  << std::endl;
			}
			else
			{
				std::cerr << "Error: Could not create destination file at " << fileToCopyDest << std::endl
						  << std::endl;
			}
		}
		break;
		case (4):
		{
			std::cout << "Enter your command: " << std::endl;
			std::string fullCommand;
			std::getline(std::cin, fullCommand);

			std::istringstream iss(fullCommand);
			std::string commandKeyword;
			iss >> commandKeyword;
			for (auto &c : commandKeyword)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

			if (commandKeyword == "ERA")
			{
				if (iss >> fileToErase)
				{
					if (!std::remove(fileToErase.c_str()))
						std::cout << "Successfully removed!" << std::endl;
					else
						std::cerr << "Error removing the file!" << std::endl;
				}
				else
				{
					std::cerr << "Usage: ERA <filename>" << std::endl;
				}
			}
			else if (commandKeyword == "PIP")
			{
				if (iss >> fileToCopy >> fileToCopyDest)
				{
					std::ifstream source(fileToCopy, std::ios::binary);
					std::ofstream dest(fileToCopyDest, std::ios::binary);

					if (source.is_open() && dest.is_open())
					{
						dest << source.rdbuf();
						std::cout << "File copied successfully to " << fileToCopyDest << "!" << std::endl;
					}
					else
					{
						std::cerr << "Error: Could not perform PIP (check file paths)." << std::endl;
					}
				}
				else
				{
					std::cerr << "Usage: PIP <source> <destination>" << std::endl;
				}
			}
			else if (commandKeyword == "MODE")
			{
				std::string target;
				if (!(iss >> target))
				{
					system("mode");
				}
				else
				{
					for (auto &c : target)
						c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

					if (target.find("CON") != std::string::npos)
					{
						system("mode CON");
						std::cout << "System display/console settings updated." << std::endl;
					}
					else
					{
						chosenModeSettings.deviceName = target;
						std::string param;
						while (iss >> param)
						{
							for (auto &c : param)
								c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
							if (param.find("BAUD=") == 0)
								chosenModeSettings.speed = std::stoi(param.substr(5));
							if (param.find("PARITY=") == 0 && param.length() >= 8)
								chosenModeSettings.parity = param[7];
							if (param.find("STOP=") == 0)
								chosenModeSettings.stopBits = std::stoi(param.substr(5));
						}
						std::cout << "Local settings for " << target << " updated." << std::endl;
						printModeSettings(chosenModeSettings);
					}
				}
			}
			else
			{
				std::cerr << "Unknown command! Use ERA, PIP or MODE." << std::endl;
			}
		}
		break;
		case (5):
			std::cout << "Goodbye, have a great one!" << std::endl;
			break;
		}
	} while (commandChosen != 5);
	return 0;
}