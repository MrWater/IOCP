#include <iostream>
#include <cstdio>

//#define NDEBUG

#include "iocp.h"
//#include "..\IOCP\iocp.h"
//
//#pragma comment(lib, "iocp.lib")

using namespace std;
using namespace iocp;

int main()
{
	CIocp iocp;
	iocp.Start(AF_INET, INADDR_ANY, htons(12345));

	string s;
	getline(cin, s);
	if (s == "q")
		iocp.Stop();

	cout << "Íê³É" << endl;

	system("pause");
	return 0;
}