// Need this to bypass the retarded main redirect logic
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <GL/glew.h>
#include <SDL.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SDL_opengl.h>


static int main_opengl();
// Should use D3D9 by default
static int main_sdl();
static int main_vbos();

#define the_main main_opengl


int main(int argc, char **argv)
{
	the_main();
}


//
// This version uses OpenGL directly to do all the drawing.
//

int main_opengl()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("sdl init failed %s\n", SDL_GetError());
		//return 1;
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

#if 0
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("window failed %s\n", SDL_GetError());
		//return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		printf("context failed %s\n", SDL_GetError());
		//return 1;
	}

	if (SDL_GL_MakeCurrent(window, context) != 0)
	{
		printf("make current failed %s \n", SDL_GetError());
		//return 1;
	}

#if 1
	// "Enable vsync"
	if (SDL_GL_SetSwapInterval(1) != 0)
	{
		printf("set swap interval failed %s\n", SDL_GetError());
		//return 1;
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



		int fc_mod4 = frame_counter % 4;
		// "Divide-remainder" by the number of colored rects you want
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

#if 0
		// Alternative way to fix it (instead of drawing many rects): Call this often
		for (int i = 0; i < 10000; ++i)
		{
			glFlush();
		}
#endif

		glFlush();
		SDL_GL_SwapWindow(window);

#if 0
		auto errors = glGetError();
		if (errors != 0)
		{
			printf("ERROR %x\r\n", (int)errors);
		}
#endif
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

	Uint32 sdl_renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
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

	SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);

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

		SDL_SetRenderTarget(renderer, texTarget);

		SDL_RenderClear(renderer);
		SDL_Rect rect1;
		rect1.x = 500;
		rect1.y = 100;
		rect1.w = 40;
		rect1.h = 30;
		SDL_RenderFillRect(renderer, &rect1);

		SDL_SetRenderTarget(renderer, NULL);

		SDL_RenderCopy(renderer, texTarget, NULL, &rect1);

		SDL_RenderPresent(renderer);
	}

_quit:

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}



const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int main_vbos()
{
	// settings

		// glfw: initialize and configure
		// ------------------------------
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		return -1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		return 1;
	}

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
		return 1;
	}

	if (SDL_GL_MakeCurrent(window, context) != 0)
	{
		return 1;
	}

#if 0
	// "Enable vsync"
	if (SDL_GL_SetSwapInterval(1) != 0)
	{
		return 1;
	}
#endif

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		return 1;
		//std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		return 1;
		//std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		return 1;
		//std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
#define NUM_TRIANGLES 10000
#define NUM_VERTICES_PER_TRIANGLE 3
#define NUM_FLOATS_PER_VERTEX 3
	float vertices[NUM_TRIANGLES * NUM_VERTICES_PER_TRIANGLE * NUM_FLOATS_PER_VERTEX];
	for (int i = 0; i < NUM_TRIANGLES; ++i)
	{
		int cursor = 0;
		vertices[i * 9 + cursor++] = 0.5f;
		vertices[i * 9 + cursor++] = 0.5f;
		vertices[i * 9 + cursor++] = 0.0f;

		vertices[i * 9 + cursor++] = 0.5f;
		vertices[i * 9 + cursor++] = -0.5f;
		vertices[i * 9 + cursor++] = 0.0f;

		vertices[i * 9 + cursor++] = -0.5f;
		vertices[i * 9 + cursor++] = -0.5f;
		vertices[i * 9 + cursor++] = 0.0f;
	}

#if 0
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
#endif
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

#if 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
#endif

	glVertexAttribPointer(0, NUM_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, NUM_FLOATS_PER_VERTEX * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);


	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int frame_counter = 0;

	// render loop
	// -----------
	while (1)
	{
		// input
		// -----
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				goto _end;
			}
		}

		++frame_counter;

		// render
		// ------
		if (frame_counter & 1)
		{
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		}
		else
		{
			glClearColor(0, 0, 0, 1.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT);

		// draw our first triangle
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

#if 1
		glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * NUM_VERTICES_PER_TRIANGLE);
#else
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
#endif
		// glBindVertexArray(0); // no need to unbind it every time 

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------

		SDL_GL_SwapWindow(window);

#if 0
		auto errors = glGetError();
		if (errors != 0)
		{
			printf("ERROR %x\r\n", (int)errors);
		}
#endif

		SDL_Delay(10);
	}

_end:

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	return 0;
}
