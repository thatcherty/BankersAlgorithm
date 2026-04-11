#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include "InputHandler.h"
#include "FtxuiIO.h"
#include "node.h"

using namespace std;

class state
{
public:
	void state_bak(int rcount, int pcount);
	state(int test = 0);
	void sim();
	~state();
private:
	vector<int> resource;
	vector<int> avail;
	vector<vector<int>> claim;
	vector<vector<int>> alloc;
	void calc_avail();
	bool is_safe(node* curr, int i);
	void calc_need(node* curr);
	bool completed();
	void run(node* curr, int i);
	bool p_done(node* curr, int p);
	int pcount;
	int rcount;
	node* head;
};

