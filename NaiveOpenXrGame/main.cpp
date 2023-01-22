#include "sources/mainCommon.h"
#include "sources/NaiveGame.h"

int main()
{
	try
	{
		Noxg::NaiveGame game{ };
		game.init();
		game.run();
	}
	catch (const std::exception& e)
	{
		LOG_FAILURE();
		LOG_ERRO(e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}