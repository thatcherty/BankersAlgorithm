#include "state.h"

state::state()
{
	head = new node();
	pcount = 0;
	rcount = 0;
}

state::state(int p, int r, int test)
{

	head = new node();

	if (test)
	{
		pcount = 4;
		rcount = 3;

		claim = {
			{3,2,2},
			{6,1,3},
			{3,1,4},
			{4,2,2}
		};

		alloc = {
			{1,0,0},
			{6,1,2},
			{2,1,1},
			{0,0,2}
		};

		resource = { 9,3,6 };

		calc_avail();

		calc_need(head);

		sim();

	}

}

void state::calc_avail()
{
	for (int r = 0; r < rcount; r++)
	{
		int sum = 0;

		for (int p = 0; p < pcount; p++)
		{
			sum += alloc[p][r];
		}

		avail.push_back(resource[r] - sum);
	}
}

bool state::is_safe(node* curr, int p)
{
	for (int r = 0; r < rcount; r++)
	{
		if (curr->need[p][r] > avail[r]) return false;
	}

	return true;
}

void state::calc_need(node* curr)
{

	for (int p = 0; p < pcount; p++)
	{
		curr->need.push_back({});
		curr->curr_claim.push_back({});
		curr->explored.push_back(false);
		for (int r = 0; r < rcount; r++)
		{
			// save current need
			curr->need[p].push_back(claim[p][r] - alloc[p][r]);

			// save current claim
			curr->curr_claim[p].push_back(claim[p][r]);
		}
	}
}

void state::run(node* curr, int p)
{
	for (int r = 0; r < rcount; r++)
	{
		avail[r] += alloc[p][r];
		alloc[p][r] = 0;
		claim[p][r] = 0;
	}
}

// check if all resources are available
bool state::completed()
{
	for (int r = 0; r < rcount; r++)
	{
		if (avail[r] != resource[r]) return false;
	}

	return true;
}

bool state::p_done(node* curr, int p)
{
	for (int r = 0; r < rcount; r++)
	{
		if (curr->need[p][r] > 0) return false;
	}

	return true;
}


void state::sim()
{
	node* curr = head;
	path = {};

	// if we traverse all n processes, we are safe
	while (path.size() != pcount && curr != nullptr)
	{
		bool safe = false;

		for (int p = 0; p < pcount; p++)
		{
			if (p_done(curr, p)) continue;

			if (!curr->explored[p] && is_safe(curr, p))
			{
				path.push_back(p);
				safe = true;
				break;
			}
		}

		if (!safe)
		{
			cout << "Unsafe, deadlock inevitable";
			return;
		}
		else
		{
			run(curr, path.back());

			if (!completed())
			{
				node* temp = new node();
				curr->explored[path.back()] = true;
				temp->parent = curr;
				curr = temp;
				curr->process = path.back();
				calc_need(curr);
			}
		}
	}

	cout << "All processes can complete" << endl;

	for (int p = 0; p < pcount; p++)
	{
		if (p + 1 == pcount)
		{
			cout << "P" << path[p] + 1 << endl;
		}
		else
		{
			cout << "P" << path[p] + 1 << "->";
		}
	}
}

state::~state()
{
	delete head;
}


// for UI
void state::set_dimensions(int resources, int processes)
{
	rcount = resources;
	pcount = processes;

	resource.assign(rcount, 0);
	avail.assign(rcount, 0);
	claim.assign(pcount, vector<int>(rcount, 0));
	alloc.assign(pcount, vector<int>(rcount, 0));
}

void state::set_resource(const vector<int>& r)
{
	resource = r;
}

void state::set_claim(const vector<vector<int>>& c)
{
	claim = c;
}

void state::set_alloc(const vector<vector<int>>& a)
{
	alloc = a;
}

const vector<int>& state::get_resource() const { return resource; }
const vector<int>& state::get_avail() const { return avail; }
const vector<vector<int>>& state::get_claim() const { return claim; }
const vector<vector<int>>& state::get_alloc() const { return alloc; }
const std::vector<int>& state::get_path() const { return path; }

bool state::validate_alloc_le_claim(vector<pair<int, int>>& bad_cells) const
{
	bad_cells.clear();

	for (int i = 0; i < pcount; ++i)
	{
		for (int j = 0; j < rcount; ++j)
		{
			if (alloc[i][j] > claim[i][j])
			{
				bad_cells.push_back({ i, j });
			}
		}
	}

	return bad_cells.empty();
}


int state::get_rcount() const {
	return rcount;
}

int state::get_pcount() const {
	return pcount;
}

void state::prepare_for_simulation()
{
	delete head;
	head = new node();

	avail.clear();
	path.clear();

	calc_avail();
	calc_need(head);
}