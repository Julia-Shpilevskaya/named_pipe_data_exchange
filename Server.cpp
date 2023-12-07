#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <algorithm>


struct employee
{
	int num;
	char name[10];
	double hours;
};


std::string file_name;
int number_of_employees;
employee* emps;
int number_of_clients;
HANDLE* hSemaphore;


DWORD WINAPI operations(LPVOID pipe)
{
	HANDLE hPipe = (HANDLE)pipe;
	DWORD dwBytesRead;
	DWORD dwBytesWrite;

	int message;
	int chosen_option;
	employee* emp_to_push;
	bool checker;
	int ID;
	bool check;
	int msg;

	while (true)
	{
		if (!ReadFile(hPipe, &message, sizeof(message), &dwBytesRead, NULL))
			return 0;

		ID = message / 10;
		chosen_option = message % 10;
		check = false;

		for (int i = 0; i < number_of_employees; i++)
		{
			if (emps[i].num == ID)
			{
				ID = i;
				check = true;
			}
		}

		WriteFile(hPipe, &check, sizeof(check), &dwBytesWrite, NULL);

		if (!check)
			continue;

		if (chosen_option == 1)
		{
			for (int i = 0; i < number_of_clients; i++)
				WaitForSingleObject(hSemaphore[ID], INFINITE);

			emp_to_push = new employee();

			emp_to_push->num = emps[ID].num;
			emp_to_push->hours = emps[ID].hours;
			strcpy_s(emp_to_push->name, emps[ID].name);

			checker = WriteFile(hPipe, emp_to_push, sizeof(employee), &dwBytesWrite, NULL);

			if (checker)
				std::cout << "Data to modify was sent.\n";
			else
				std::cout << "Data to modify wasn't sent.\n";

			ReadFile(hPipe, emp_to_push, sizeof(employee), &dwBytesWrite, NULL);

			emps[ID].hours = emp_to_push->hours;
			strcpy_s(emps[ID].name, emp_to_push->name);

			std::ofstream fout;
			fout.open(file_name);

			for (int i = 0; i < number_of_employees; i++)
				fout << emps[i].num << " " << emps[i].name << " " << emps[i].hours << "\n";

			fout.close();

			ReadFile(hPipe, &msg, sizeof(msg), &dwBytesWrite, NULL);

			if (msg == 1)
				for (int i = 0; i < number_of_clients; i++)
					ReleaseSemaphore(hSemaphore[ID], 1, NULL);
		}
		else if (chosen_option == 2)
		{
			WaitForSingleObject(hSemaphore[ID], INFINITE);

			emp_to_push = new employee();
			emp_to_push->num = emps[ID].num;
			emp_to_push->hours = emps[ID].hours;
			strcpy_s(emp_to_push->name, emps[ID].name);

			checker = WriteFile(hPipe, emp_to_push, sizeof(employee), &dwBytesWrite, NULL);

			if (checker)
				std::cout << "Data to read was sent.\n";
			else
				std::cout << "Data to read wasn't sent.\n";

			ReadFile(hPipe, &msg, sizeof(msg), &dwBytesWrite, NULL);

			if (msg == 1)
				ReleaseSemaphore(hSemaphore[ID], 1, NULL);
		}
	}

	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
}


int main()
{
	std::cout << "Enter file name:\n";
	std::cin >> file_name;

	std::cout << "Enter number of employees:\n";
	std::cin >> number_of_employees;

	emps = new employee[number_of_employees];

	for (int i = 0; i < number_of_employees; i++)
	{
		std::cout << "Enter " << i + 1 << " employee ID:\n";
		std::cin >> emps[i].num;

		std::cout << "Enter employee name:\n";
		std::cin >> emps[i].name;

		std::cout << "Enter employee hours:\n";
		std::cin >> emps[i].hours;
	}

	std::ofstream fout;
	fout.open(file_name);

	for (int i = 0; i < number_of_employees; i++)
		fout << emps[i].num << " " << emps[i].name << " " << emps[i].hours << "\n";

	fout.close();

	std::ifstream fin;
	fin.open(file_name);

	int ID;
	std::string name;
	double hours;

	for (int i = 0; i < number_of_employees; i++)
	{
		fin >> ID >> name >> hours;
		std::cout << "\nID of employee: " << ID << "\nName of employee: " << name << "\nHours of employee: " << hours << "\n";
	}

	fin.close();

	std::cout << "Enter number of clients:\n";
	std::cin >> number_of_clients;

	HANDLE* hStarted = new HANDLE[number_of_clients];
	hSemaphore = new HANDLE[number_of_employees];

	for (int i = 0; i < number_of_employees; i++)
		hSemaphore[i] = CreateSemaphore(NULL, number_of_clients, number_of_clients, L"hSemahpore");

	for (int i = 0; i < number_of_clients; ++i)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		std::string cmd = "Named_pipe_client.exe";
		std::wstring str_to_wstr = std::wstring(cmd.begin(), cmd.end());
		LPWSTR clientCmdLine = &str_to_wstr[0];
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		CreateProcess(NULL, clientCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);

		hStarted[i] = CreateEvent(NULL, FALSE, FALSE, L"Process Started");

		CloseHandle(pi.hProcess);
	}

	WaitForMultipleObjects(number_of_clients, hStarted, TRUE, INFINITE);

	HANDLE* hPipe = new HANDLE[number_of_clients];
	HANDLE* hThreads = new HANDLE[number_of_clients];

	for (int i = 0; i < number_of_clients; i++)
	{
		hPipe[i] = CreateNamedPipe(L"\\\\.\\pipe\\pipe_name", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
			0, 0, INFINITE, NULL);

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			std::cout << "Creation of the named pipe failed.\n The last error code: " << GetLastError() << "\n";
			std::cout << "Press any char to finish server: ";
			_getch();
			return 0;
		}

		if (!ConnectNamedPipe(hPipe[i], (LPOVERLAPPED)NULL))
		{
			std::cout << "The connection failed.\nThe last error code: " << GetLastError() << "\n";
			CloseHandle(hPipe[i]);

			std::cout << "Press any char to finish the server: ";
			_getch();
			return 0;
		}

		hThreads[i] = CreateThread(NULL, 0, operations, static_cast<LPVOID>(hPipe[i]), 0, NULL);
	}

	WaitForMultipleObjects(number_of_clients, hThreads, TRUE, INFINITE);

	std::cout << "All clients has ended their work.";

	fin.open(file_name);

	for (int i = 0; i < number_of_employees; i++)
	{
		fin >> ID >> name >> hours;
		std::cout << "\nID of employee: " << ID << "\nName of employee: " << name << "\nHours of employee: " << hours << "\n";
	}

	fin.close();

	std::cout << "Press any char to finish the server: \n";
	_getch();

	return 0;
}