#include "state.h"

state::state(int test)
{
	FtxuiIO ui;

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
	else
	{

		string prompt = "Number of available resources: ";
		rcount = ui.PromptInt(prompt);

		prompt = "Please enter a process count: ";
		pcount = ui.PromptInt(prompt);
	}
}

void state::state_bak(int rcount, int pcount)
{
	// input resource vector
	for (int i = 0; i < rcount; i++)
	{
		string prompt = "Please enter a resource count for resource " + to_string(i + 1);

		int temp = InputHandler::GetInt(&prompt);

		resource.push_back(temp);
	}

	// input resource vector
	for (int i = 0; i < pcount; i++)
	{
		claim.push_back({});
		for (int j = 0; j < rcount; j++)
		{
			string prompt = "Please enter the resource claim for process " + to_string(i + 1) + " and resource " + to_string(j + 1);

			int temp = InputHandler::GetInt(&prompt);

			// check that claim is not greater than max resource allocation
			if (temp > resource[j])
			{
				cout << "Process " << i + 1 << " with claim of " << temp << " exceeds value for resource " << j + 1 << " with only " << resource[j] << " resource's to allocate." << endl;

				// exit program
				return;
			}

			claim[i].push_back(temp);
		}

	}

	// input current allocation state
	for (int i = 0; i < pcount; i++)
	{
		alloc.push_back({});
		for (int j = 0; j < rcount; j++)
		{
			string prompt = "Please enter the resource allocation for process " + to_string(i + 1) + " and resource " + to_string(j + 1);

			int temp = InputHandler::GetInt(&prompt);

			// check that allocation is not greater than the claim
			if (temp > claim[i][j])
			{
				cout << "Process " << i + 1 << " with claim of " << temp << " exceeds value for resource " << j + 1 << " with only " << claim[i][j] << " resource's to allocate." << endl;

				// exit program 
				return;
			}

			claim[i].push_back(temp);
		}

	}

	// calculate available vector
	calc_avail();

	// simulate process execution and resource alloc
	sim();

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
	vector<int> path{};

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
