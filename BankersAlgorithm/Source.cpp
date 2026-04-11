#include "BankersFTXApp.h"
#include "state.h"


using namespace std;

int main()
{
	int test = 0;
	if (test)
	{
		state(4, 3, 1);
	}
	else
	{
		state sim_state;
		BankersFTXApp app(sim_state);
		app.Run();
	}

	return 0;
}