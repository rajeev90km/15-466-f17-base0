#include "Draw.hpp"
#include "GL.hpp"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <chrono>
#include <iostream>
#include <list>

enum BaseColors{BLACK,GRAY,YELLOW,PURPLE,RED};

glm::u8vec4 BaseColorValues[5] = {glm::u8vec4(0x1f, 0x1f, 0x1f, 0xff),glm::u8vec4(0xf2, 0xf2, 0xf2, 0xff),glm::u8vec4(0xff, 0xd9, 0x1e, 0xff),glm::u8vec4(0xaa, 0x61, 0xce, 0xff),glm::u8vec4(0xe7, 0x59, 0x26, 0xff)};

class Base{
    public:
        float x1,y1,x2,y2;
        BaseColors base_color;
        glm::u8vec4 base_color_val;
};

class Missile{
    public:
        float x1 = 0.0f;
        float y1 = 0.0f;
        float x2 = 0.1f;
        float y2 = 0.07f;
        glm::vec2 position;
        glm::vec2 velocity;
        BaseColors missile_color;
        glm::u8vec4 missile_color_val;
};

std::list<Missile> missiles;
bool game_over_flag = false;
float randVelocities[] = {-0.8,0.8};

Uint32 spawn_missile(Uint32 interval, void *param){
    
    
    int colorIndex = rand() % 5;
    
    float randX = (rand()%19 - 9);
    float XVel = randVelocities[rand()%2];
    
    
    
    Missile m;
    m.position = glm::vec2(randX/10, 1.0f);
    m.velocity = glm::vec2(XVel, -0.8f);
    m.missile_color = (BaseColors) colorIndex;
    m.missile_color_val = BaseColorValues[colorIndex];
    
    missiles.push_back(m);
    
    return interval;
}

int main(int argc, char **argv) {
    
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
//	glm::vec2 ball = glm::vec2(0.0f, 0.8f);
//	glm::vec2 ball_velocity = glm::vec2(0.9f, -0.9f);
    
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
        
    
    //------------Spawn Missiles--------------------
    Uint32 missile_delay = 1750;  /* To round it down to the nearest 10 ms */
    SDL_TimerID missile_timer = SDL_AddTimer(missile_delay, spawn_missile, NULL);
    

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
            if(game_over_flag==false){
                std::list<Missile>::iterator m;
                for (m = missiles.begin(); m != missiles.end(); m++){
                    m->position += elapsed * m->velocity;
                
                    if (m->position.x < -1.0f) m->velocity.x = std::abs(m->velocity.x);
                    if (m->position.x >  1.0f) m->velocity.x =-std::abs(m->velocity.x);
                
                    if (m->position.y >  1.0f) m->velocity.y =-std::abs(m->velocity.y);
                
                    //Check for collision
                    for(int i=0;i<numUnits;i++){
                        if (m->position.y <  baseUnits[i].y2){
                            if(m->position.x >= (mouse.x+baseUnits[i].x1)
                               && m->position.x <= (mouse.x+baseUnits[i].x2)){
                                
                                //Wrong catch
                                if(m->missile_color!=baseUnits[i].base_color){
                                    missiles.pop_front();
                                    baseUnits[i].y2 -= 0.15;
                                }
                                else{
                                    missiles.pop_front();
                                }
                            }
                        }
                    }
                    
                    if (m->position.y < -1.0f) {
                        game_over_flag = true;
                        SDL_RemoveTimer(missile_timer);
                    }
                }
            }
		}

		//draw output:
        
        glClearColor(0.2,0.2,0.2,0.2);
        glClear(GL_COLOR_BUFFER_BIT);
        


		{ //draw game state:
            if(game_over_flag==false){
                Draw draw;
            
                // Draw base
                for(int i=0;i<numUnits;i++){
                    draw.add_rectangle(mouse+glm::vec2(baseUnits[i].x1,baseUnits[i].y1),mouse+glm::vec2(baseUnits[i].x2,baseUnits[i].y2),baseUnits[i].base_color_val);
                }
            
            
                //Draw Missiles
                for (std::list<Missile>::iterator m = missiles.begin(); m != missiles.end(); m++){
                    draw.add_rectangle(m->position + glm::vec2(m->x1,m->y1), m->position + glm::vec2(m->x2, m->y2), m->missile_color_val);
                }
                draw.draw();
            }
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
