#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <string.h>
#include <sstream>
#include <Windows.h>
#include <mutex>
#include <chrono>


#pragma warning(disable : 4996)

using namespace std;

mutex mtx1;
mutex mtx2;

class Stroka
{
public:
	string arr;
	void Edit();
	void EditToKB();
	bool exam();
};

void Stroka::Edit()
{
	do
	{
		arr = '0';
		cout << "Please, input string of digits\n";
		cin >> arr;
	} while (!exam());
}

bool Stroka::exam()
{
	size_t size = 64;
	if (arr.length() > size)
	{
		return false;
	}
	for (char ch : arr)
	{
		if (!isdigit(ch))
		{
			return false;
		}
	}
	return true;
}

void Stroka::EditToKB()
{
	for (int j = 0; j < arr.length(); j++)
	{
		if ((arr[j]) % 2 == 0 && arr[j] != '0')
		{
			arr.append("0");
			for (int i = arr.length(); i > j; i--)
			{
				arr[i] = arr[i - 1];
			}
			arr[j] = 'K';
			arr[j + 1] = 'B';
			j++;
		}
	}
}

int Sum(string& arr)
{
	int sum = 0;

	for (char ch : arr)
	{
		if (isdigit(ch))
		{
			sum += ch - '0';
		}
	}
	return sum;
}

bool comp(char a, char b)
{
	return a > b;
}

void toClipboard(Stroka& s)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.arr.size() + 1);
		if (!hg)
		{
			CloseClipboard();
			return;
		}
		memcpy(GlobalLock(hg), s.arr.c_str(), s.arr.size() + 1);
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		GlobalFree(hg);
	}
}

void fromClipboard(string& arr2)
{
	if (OpenClipboard(0))
	{
		HGLOBAL hData = GetClipboardData(CF_TEXT);
		char* chBuffer = (char*)GlobalLock(hData);
		if (chBuffer != NULL)
		{
			arr2 = chBuffer;
		}
		GlobalUnlock(hData);
		EmptyClipboard();
		CloseClipboard();
	}
}

void Thread1(Stroka& s)
{
	while (true)
	{
		mtx1.lock();
		s.Edit();
		mtx1.unlock();
		sort(s.arr.begin(), s.arr.end(), comp);
		s.EditToKB();
		toClipboard(s);
	}
}

void Thread2(string& arr2, SOCKET& newConnection, SOCKET& sListen, SOCKADDR_IN& addr, int& sizeofaddr)
{
	newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
	while (true)
	{
		int sum = 0;
		char buff[20];
		char* p;
		mtx1.lock();
		fromClipboard(arr2);
		sum = Sum(arr2);
		if (arr2 != "0")
		{
			cout << arr2 << endl;
			p = itoa(sum, buff, 10);
			if (send(newConnection, NULL, NULL, NULL) != -1)
			{
				send(newConnection, p, sizeof(sum), NULL);
				arr2 = "0";
				mtx1.unlock();
				continue;
			}
			else
			{
				mtx1.unlock();
				cout << "The second program is disabled" << endl;
				newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
				continue;
			}
		}
		mtx1.unlock();
	}
}

int main()
{
	setlocale(LC_ALL, "rus");
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1703);
	addr.sin_family = AF_INET;


	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;

	string arr2 = "0";
	Stroka s;

	if (OpenClipboard(0))
	{
		EmptyClipboard();
		CloseClipboard();
	}
	thread th1(Thread1, std::ref(s));
	thread th2(Thread2, std::ref(arr2), std::ref(newConnection), std::ref(sListen), std::ref(addr), std::ref(sizeofaddr));
	th1.join();
	th2.join();

	return 0;
}