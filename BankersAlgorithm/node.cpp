#include "node.h"

node::node()
{
	branches = { nullptr };
	parent = nullptr;
}

node::~node()
{
	for (int i = 0; i < branches.size(); i++)
	{
		delete branches[i];
	}
}
