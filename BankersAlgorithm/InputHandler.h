#pragma once
#include <iostream>
#include <limits>
#include <string>

using namespace std;

class InputHandler
{
public:
	static int GetInt(string* prompt);
	static char GetChar(string* prompt);
	static string GetString(string* prompt);
};

