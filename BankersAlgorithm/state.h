#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include "node.h"

using namespace std;

class state
{
public:
	state(int p, int r, int test = 0);
	state();
	void sim();
	~state();


	// for UI
	void set_dimensions(int resources, int processes);
	void set_resource(const vector<int>& r);
	void set_claim(const vector<vector<int>>& c);
	void set_alloc(const vector<vector<int>>& a);

	const vector<int>& get_resource() const;
	const vector<int>& get_avail() const;
	const vector<vector<int>>& get_claim() const;
	const vector<vector<int>>& get_alloc() const;
	void prepare_for_simulation();
	const std::vector<int>& get_path() const;
	int get_rcount() const;
	int get_pcount() const;

	bool validate_alloc_le_claim(vector<pair<int, int>>& bad_cells) const;

private:
	vector<int> resource;
	vector<int> avail;
	vector<vector<int>> claim;
	vector<vector<int>> alloc;
	vector<int> path;
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

