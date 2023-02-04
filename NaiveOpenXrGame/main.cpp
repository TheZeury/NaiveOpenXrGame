#include "sources/mainCommon.h"
#include "sources/NaiveGame.h"

int main()
{
	try
	{
		Noxg::hd::GameInstance game = std::make_shared<Noxg::NaiveGame>();
		game->Initialize();
		game->Run();
	}
	catch (const std::exception& e)
	{
		LOG_FAILURE();
		LOG_ERRO(e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}