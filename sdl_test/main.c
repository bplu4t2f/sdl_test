// Need this to bypass the retarded main redirect logic
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <SDL.h>
#include <Windows.h>
#include <SDL_opengl.h>



int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		return 1;
	}

	if (SDL_GL_MakeCurrent(window, context) != 0)
	{
		return 1;
	}

#if 1
	// "Enable vsync"
	if (SDL_GL_SetSwapInterval(1) != 0)
	{
		return 1;
	}
#endif

	printf("Init OK\r\n");

	int frame_counter = 0;

	Uint64 performance_frequency = SDL_GetPerformanceFrequency();
	Uint64 last_tick = SDL_GetPerformanceCounter();

	while (1)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				goto _quit;
			}
		}

		++frame_counter;

		{
			Uint64 tmp = last_tick;
			last_tick = SDL_GetPerformanceCounter();
			Uint64 elapsed = last_tick - tmp;
			double elapsed_ms = (double)elapsed * 1000.0 / (double)performance_frequency;
			if (elapsed_ms < 15.0 || elapsed_ms > 17.0)
			{
				printf("%.3lf\r\n", elapsed_ms);
			}
		}

		int fc_mod4 = frame_counter % 4;
		int fc_mod8 = frame_counter % 8;

		if (frame_counter & 1)
		{
			glClearColor(60.0f / 255.0f, 50.0f / 255.0f, 75.0f / 255.0f, 0.0f);
		}
		else
		{
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1
		// Draw 8 square columns:

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);

		float r = fc_mod4 == 0 || fc_mod4 == 3 ? 1.0f : 0.0f;
		float g = fc_mod4 == 0 || fc_mod4 == 2 ? 1.0f : 0.0f;
		float b = fc_mod4 == 1 ? 255.0f : 0.0f;
		//glColor3f(r, g, b);
		glColor3f(r, g, b);
		glBegin(GL_QUADS);
		for (int i = 0; i < 20; ++i)
		{
			glVertex3f(10 + fc_mod8 * 20, 10 + i * 20, 0.0f);
			glVertex3f(10 + fc_mod8 * 20, 20 + i * 20, 0.0f);
			glVertex3f(20 + fc_mod8 * 20, 20 + i * 20, 0.0f);
			glVertex3f(20 + fc_mod8 * 20, 10 + i * 20, 0.0f);
		}
		glEnd();
		glFlush();
#endif

		SDL_GL_SwapWindow(window);
	}

_quit:

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
