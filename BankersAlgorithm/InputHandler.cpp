#include "InputHandler.h"

int InputHandler::GetInt(string* prompt)
{
	int temp = 0;

	cout << *prompt << endl;

	while (!(cin >> temp))
	{
		cout << "Please enter a valid integer: " << endl;
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}

	// protects congruent use with GetString
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	return temp;
}

char InputHandler::GetChar(string* prompt)
{
	char temp = '\0';

	cout << *prompt << endl;

	cin >> temp;

	while (cin.peek() != '\n')
	{
		cout << "Please enter a single character." << endl;
		temp = '\0';
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		cin >> temp;
	}
	
	return temp;
}

string InputHandler::GetString(string* prompt)
{
	string temp = "";
	cout << *prompt << endl;
	getline(cin, temp);

	return temp;
}
