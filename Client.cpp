#include <conio.h>
#include <windows.h>
#include <iostream>
#include<string>


struct employee
{
	int num;
	char name[10];
	double hours;
};


int main(int argc, char* argv)
{
	HANDLE hStartEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"Process Started");

	if (hStartEvent == NULL)
	{
		std::cout << "Open event failed. \nEnter any char to exit.\n";
		_getch();
		return GetLastError();
	}

	SetEvent(hStartEvent);

	HANDLE hPipe = CreateFile(L"\\\\.\\pipe\\pipe_name", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "Creation of the named pipe failed.\n The last error code: " << GetLastError() << "\n";
		std::cout << "Press any char to finish server: ";
		_getch();
		return 0;
	}

	std::string chosen_option;
	DWORD dwBytesWritten;
	DWORD dwBytesReaden;
	int ID;
	int message_to_send;
	bool checker;
	employee* emp;

	while (true)
	{
		ID = 0;
		std::cout << "Choose option:\n 1. Modify data\n 2. Read data\n 3. Exit\n";
		std::cin >> chosen_option;

		if (chosen_option == "1")
		{
			std::cout << "Enter ID of employee:\n";
			std::cin >> ID;

			message_to_send = ID * 10 + std::stoi(chosen_option);

			checker = WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);

			if (checker)
				std::cout << "Message was sent.\n";
			else
				std::cout << "Message wasn't sent.\n";

			bool check_is_correct;

			ReadFile(hPipe, &check_is_correct, sizeof(check_is_correct), &dwBytesReaden, NULL);

			if (!check_is_correct)
			{
				std::cout << "ID is incorrect. Try again.\n";
				continue;
			}

			emp = new employee();

			if (!ReadFile(hPipe, emp, sizeof(employee), &dwBytesReaden, NULL))
			{
				std::cout << "Data reading from the named pipe failed.\n" << "The last error code: " << GetLastError() << "\n";
				std::cout << "Press any char to finish server: \n";
				_getch();
				return 0;
			}

			std::cout << "ID of employee: " << emp->num << "\nName of employee: " << emp->name << "\nHours of employee: " << emp->hours << "\n";

			std::cout << "Enter new Name:\n";
			std::cin >> emp->name;

			std::cout << "Enter new Hours:\n";
			std::cin >> emp->hours;

			std::cout << "Press any key to send modified record to Server\n";
			_getch();

			checker = WriteFile(hPipe, emp, sizeof(employee), &dwBytesWritten, NULL);

			if (checker)
				std::cout << "Message was sent.\n";
			else
				std::cout << "Message wasn't sent.\n";

			std::cout << "Press any key to end the selected option\n";
			_getch();

			message_to_send = 1;
			WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);
		}
		else if (chosen_option == "2")
		{

			std::cout << "Enter ID of employee:\n";
			std::cin >> ID;

			message_to_send = ID * 10 + std::stoi(chosen_option);
			checker = WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);

			if (checker)
				std::cout << "Message was sent.\n";
			else
				std::cout << "Message wasn't sent.\n";

			bool check_is_correct;

			ReadFile(hPipe, &check_is_correct, sizeof(check_is_correct), &dwBytesReaden, NULL);

			if (!check_is_correct)
			{
				std::cout << "ID is incorrect. Try again.\n";
				continue;
			}

			emp = new employee();

			if (!ReadFile(hPipe, emp, sizeof(employee), &dwBytesReaden, NULL))
			{
				std::cout << "Data reading from the named pipe failed.\n" << "The last error code: " << GetLastError() << "\n";
				std::cout << "Press any char to finish server: \n";
				_getch();
				return 0;
			}

			std::cout << "ID of employee: " << emp->num << "\nName of employee: " << emp->name << "\nHours of employee: " << emp->hours << "\n";
			std::cout << "Press any key to end the selected optionn\n";
			_getch();

			message_to_send = 1;
			WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);
		}
		else  if (chosen_option == "3")
			break;
		else
			std::cout << "Incorrect input.Try again\n";
	}

	return 0;
}