#include "Draw.hpp"
#include "GL.hpp"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <chrono>
#include <iostream>
#include <queue>

enum BaseColors{BLACK,GRAY,YELLOW,PURPLE,RED};

glm::u8vec4 BaseColorValues[5] = {glm::u8vec4(0x3f, 0x3f, 0x3f, 0xff),glm::u8vec4(0xf2, 0xf2, 0xf2, 0xff),glm::u8vec4(0xff, 0xd9, 0x1e, 0xff),glm::u8vec4(0xaa, 0x61, 0xce, 0xff),glm::u8vec4(0xe7, 0x59, 0x26, 0xff)};

class Base{
    public:
        float x1,y1,x2,y2;
        BaseColors base_color;
        glm::u8vec4 base_color_val;
};



int main(int argc, char **argv) {
    
    std::queue<int> missiles;
    missiles.push(1);
    
    std::cout << missiles.front();
    
	//Configuration:
	struct {
		std::string title = "Game0: Color slide";
		glm::uvec2 size = glm::uvec2(480, 640);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}
    
    

	//Hide mouse cursor (note: showing can be useful for debugging):
//	SDL_ShowCursor(SDL_DISABLE);

	//------------  game state ------------

	glm::vec2 mouse = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity = glm::vec2(0.9f, -0.9f);
    
    const glm::vec2 start_pos = glm::vec2(-3.0f,-1.0f);
    float init_base_height = -0.55f;
    const int numUnits = 10;
    Base baseUnits[numUnits];
    
    
    //----------- Create Base Units ---------START
    {

        for(int i=0;i<numUnits;i++){
            baseUnits[i].x1 = start_pos.x + (i*0.5f);
            baseUnits[i].x2 = baseUnits[i].x1 + 0.5f;
        
            baseUnits[i].y1 = start_pos.y;
            baseUnits[i].y2 = init_base_height;
        
            baseUnits[i].base_color = (BaseColors) (i%5);
            baseUnits[i].base_color_val = BaseColorValues[baseUnits[i].base_color];
        }
    }
     //----------- Create Base Units ---------END
        
        

	//------------  game loop ------------

	auto previous_time = std::chrono::high_resolution_clock::now();
	bool should_quit = false;
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
            
            if(evt.type == SDL_MOUSEMOTION) {
                    mouse.x = (evt.motion.x) / float(config.size.x) * 3.0f - 1.0f;
                    mouse.y = 0.0f;
            }
//            if (evt.type == SDL_MOUSEBUTTONDOWN) {
//                mouse_pressed = true;
//			}
//            else if(evt.type == SDL_MOUSEBUTTONUP){
//                mouse_pressed = false;
//            }
            else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
			ball += elapsed * ball_velocity;
			if (ball.x < -1.0f) ball_velocity.x = std::abs(ball_velocity.x);
			if (ball.x >  1.0f) ball_velocity.x =-std::abs(ball_velocity.x);
			if (ball.y < -1.0f) ball_velocity.y = std::abs(ball_velocity.y);
			if (ball.y >  1.0f) ball_velocity.y =-std::abs(ball_velocity.y);
		}

		//draw output:
		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);


		{ //draw game state:
			Draw draw;
            
            for(int i=0;i<numUnits;i++){
                draw.add_rectangle(mouse+glm::vec2(baseUnits[i].x1,baseUnits[i].y1),mouse+glm::vec2(baseUnits[i].x2,baseUnits[i].y2),baseUnits[i].base_color_val);
            }
//			draw.add_rectangle(glm::vec2(-1.0f,-1.0f), glm::vec2(-0.5f,-0.55f), glm::u8vec4(0x3f, 0x3f, 0x3f, 0xff));
//            draw.add_rectangle(glm::vec2(-0.5f,-1.0f), glm::vec2(0.0f,-0.55f), glm::u8vec4(0xf2, 0xf2, 0xf2, 0xff));
//            draw.add_rectangle(glm::vec2(0.0f,-1.0f), glm::vec2(0.5f,-0.55f), glm::u8vec4(0xff, 0xd9, 0x1e, 0xff));
//            draw.add_rectangle(glm::vec2(0.5f,-1.0f), glm::vec2(1.0f,-0.55f), glm::u8vec4(0xaa, 0x61, 0xce, 0xff));
            draw.add_rectangle(ball + glm::vec2(-0.04f,-0.04f), ball + glm::vec2(0.07f, 0.04f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
			draw.draw();
		}


		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
