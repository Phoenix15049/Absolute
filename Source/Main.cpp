#include <string>
#include <SDL.h>
#include "Core/Application.h"

int main(int argc, char* argv[])
{
	Application application;

	if (!application.Initialize())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Absolute Engine",
			"The engine could not start. Check the console output for details.", nullptr);
		return -1;
	}

	for (int i = 1; i < argc; i++)
	{
		const std::string argument = argv[i];

		if (argument == "--capture" && i + 1 < argc)
		{
			application.CaptureWindowAndQuit(argv[++i]);
		}

		else
		{
			application.OpenScene(argument);
		}
	}

	application.Run();
	application.Shutdown();

	return 0;
}
