// Need this to bypass the retarded main redirect logic
#define SDL_MAIN_HANDLED

#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <SDL.h>
#include <Windows.h>
#include <SDL_opengl.h>

#include <gl/GL.h>
#include <gl/GLU.h>


static int main_opengl();
static int main_sdl();


int main(int argc, char **argv)
{
#if 1
	main_opengl();
#else
	// Should use D3D9 by default
	main_sdl();
#endif
}


//
// This version uses OpenGL directly to do all the drawing.
//

int main_opengl()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return 1;
	}

#if 0
	// This block seems to have no effect on the problem
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

#if 0
	// Enabling multisampling seems to improve the issue, but it's not good enough yet.
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); // @TODO doesn't seem to be necessary
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#endif

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

		//frame_counter = 0;

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



#if 0
		// This block is stolen from obbg (https://github.com/nothings/obbg)
		// See main.c line 600 and following
		// If we can get something equivalent working, the problem should be alleviated significantly.

		for (distance = 0; distance <= rad; ++distance) {
			for (j = -distance; j <= distance; ++j) {
				for (i = -distance; i <= distance; ++i) {
					int cx = qchunk_x + i;
					int cy = qchunk_y + j;
					int slot_x = cx & (C_MESH_CHUNK_CACHE_X - 1);
					int slot_y = cy & (C_MESH_CHUNK_CACHE_Y - 1);
					mesh_chunk *mc = c_mesh_cache[slot_y][slot_x];

					if (stb_max(abs(i), abs(j)) != distance)
						continue;

					if (i*i + j*j > rad*rad)
						continue;

					if (mc == NULL || mc->placeholder_for_size_info || mc->chunk_x != cx || mc->chunk_y != cy || mc->vbuf == 0) {
						float estimated_bounds[2][3];
						if (num_build_remaining == 0)
							continue;
						estimated_bounds[0][0] = (float)(cx << MESH_CHUNK_SIZE_X_LOG2);
						estimated_bounds[0][1] = (float)(cy << MESH_CHUNK_SIZE_Y_LOG2);
						estimated_bounds[0][2] = (float)(0);
						estimated_bounds[1][0] = (float)((cx + 1) << MESH_CHUNK_SIZE_X_LOG2);
						estimated_bounds[1][1] = (float)((cy + 1) << MESH_CHUNK_SIZE_Y_LOG2);
						estimated_bounds[1][2] = (float)(255);
						if (!is_box_in_frustum(estimated_bounds[0], estimated_bounds[1]))
							continue;
						mc = build_mesh_chunk_for_coord(mc, cx * C_MESH_CHUNK_CACHE_X, cy * C_MESH_CHUNK_CACHE_Y);
						--num_build_remaining;
					}

					++chunk_locations;

					++chunks_considered;
					quads_considered += mc->num_quads;
					chunk_storage_considered += mc->num_quads * 20;

					if (mc->num_quads && !mc->placeholder_for_size_info) {
						if (is_box_in_frustum(mc->bounds[0], mc->bounds[1])) {
							// @TODO if in range, frustum cull
							stbglUniform3fv(stbgl_find_uniform(main_prog, "transform"), 3, mc->transform[0]);
							glBindBufferARB(GL_ARRAY_BUFFER_ARB, mc->vbuf);
							glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 4, (void*)0);
							glBindTexture(GL_TEXTURE_BUFFER_ARB, mc->fbuf_tex);

							glDrawArrays(GL_QUADS, 0, mc->num_quads * 4);

							quads_rendered += mc->num_quads;
							++chunks_in_frustum;
							chunk_storage_rendered += mc->num_quads * 20;
						}
					}
				}
			}
		}
#endif

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
		float g = fc_mod4 != 3 ? 1.0f : 0.0f;
		float b = fc_mod4 == 1 ? 1.0f : 0.0f;
		glColor3f(r, g, b);

		// This is ridiculous - if we render 20000 rects, everything is OK, if we render
		// only 20, everything is horrible.
		int num_rects = ((frame_counter / 120) & 1) ? 20000 : 20;

		glBegin(GL_QUADS);
		for (int i = 0; i < num_rects; ++i)
		{
			glVertex3f(10.0f + fc_mod8 * 20.0f, 10.0f + i * 20, 0.0f);
			glVertex3f(10.0f + fc_mod8 * 20.0f, 20.0f + i * 20, 0.0f);
			glVertex3f(20.0f + fc_mod8 * 20.0f, 20.0f + i * 20, 0.0f);
			glVertex3f(20.0f + fc_mod8 * 20.0f, 10.0f + i * 20, 0.0f);
		}

		glEnd();
#endif

		
		glFlush();
		SDL_GL_SwapWindow(window);
	}

_quit:

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


//
// This version uses SDL to do all the drawing, and doesn't use any OpenGL API directly
//

int main_sdl()
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

	int selected_renderer_index = -1;
	int num_render_drivers = SDL_GetNumRenderDrivers();
	for (int i = 0; i < num_render_drivers; ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		printf("Available renderer %d: %s\r\n", i, info.name);
#if 1
		// Use #if 0 to use the default, which should be d3d, and #if 1 to use Open GL with SDL abstraction.
		if (!strcmp("opengl", info.name))
		{
			printf("^ using this renderer\r\n");
			selected_renderer_index = i;
		}
#endif
	}

	Uint32 sdl_renderer_flags = SDL_RENDERER_ACCELERATED;
#if 1
	// enable vsync
	sdl_renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
#endif
	SDL_Renderer *renderer = SDL_CreateRenderer(window, selected_renderer_index, sdl_renderer_flags);
	if (renderer == NULL)
	{
		return 1;
	}

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

		//frame_counter = 0;

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
			SDL_SetRenderDrawColor(renderer, 60, 50, 75, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		}
		SDL_RenderClear(renderer);

#if 1
		// Draw 8 square columns:
#define RECT_COUNT 20
		SDL_Rect rects[RECT_COUNT];
		Uint8 r = fc_mod4 == 0 || fc_mod4 == 3 ? 255 : 0;
		Uint8 g = fc_mod4 != 3 ? 255 : 0;
		Uint8 b = fc_mod4 == 1 ? 255 : 0;
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
		for (int i = 0; i < RECT_COUNT; ++i)
		{
			rects[i].x = 10 + fc_mod8 * 20;
			rects[i].y = 10 + i * 20;
			rects[i].w = 10;
			rects[i].h = 10;
		}
		SDL_RenderFillRects(renderer, rects, RECT_COUNT);
#endif

		SDL_RenderPresent(renderer);
	}

_quit:

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
