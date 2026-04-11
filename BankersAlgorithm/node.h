#pragma once
#include <iostream>
#include <vector>

using namespace std;

class node
{
public:
	node();
	node* parent;
	vector<node*> branches;
	vector<vector<int>> need;
	vector<vector<int>> curr_claim;
	vector<bool> explored;
	int process;
	~node();
};