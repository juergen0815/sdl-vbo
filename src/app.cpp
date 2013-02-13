/*
 * app.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "err.h"
#include "app.h"
#include "renderer.h"

#include "viewport.h"
#include "camera.h"
#include "cube.h"
#include "sphere.h"

#include <SDL/SDL.h>

#include <GL/glew.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

App::App()
    : m_Worker(new Renderer)
{
}

App::~App()
{
}

void App::Init(int argc, char* argv[])
{
    int err = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    ASSERT( err != -1, "Failed to initialize SDL video system! SDL Error: %s\n", SDL_GetError());

    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    int stencilSize(8);
    err = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilSize);
    ASSERT( err != -1, "Error setting stencil size to %d! SDL Error:  %s\n", stencilSize, SDL_GetError());
    // enable multi sampling
    err = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    ASSERT( err != -1, "Error enabling multi sampling! SDL Error: %s\n", SDL_GetError());

    int numSampleBuffers(2); // test what's the max AA. test 8xMSAA
    err = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, numSampleBuffers);
    ASSERT( err != -1, "Error setting number (%d) of AA buffer! SDL Error: %s\n", numSampleBuffers, SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int vsync = 1;  // 0 = novsync
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);

    SDL_WM_SetCaption("SDL VBO Example", NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    //  SDL_WM_SetIcon( pei::SDL::SurfaceRef( pei::LogoBrush ), NULL );
    //	SDL_ShowCursor(SDL_DISABLE);
    //	SDL_EnableUNICODE(1);

    int numJoysticks = SDL_NumJoysticks();
    if ( numJoysticks > 0 ) {
        m_Joystick = SDL_JoystickOpen(0);
        SDL_JoystickEventState(SDL_ENABLE);
    }

}

int App::Run()
{
    int r(0);

    // somebody must attach a worker
    BOOST_ASSERT( m_Worker);

    int width(640);
    int height(480);
    SDL_Surface *screen;
    screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
    if (screen == NULL)
    {
        THROW("Unable to set %dx%d video! SDL Error: %s\n", width, height, SDL_GetError());
    }

    Renderer* renderer = dynamic_cast<Renderer*>(m_Worker.get());
    BOOST_ASSERT(renderer);
    renderer->Init();

    ////////////////////////////////////////////////////////////////////////////
    // Compose our scene
    int order(0);

    // Add a cube
    EntityPtr viewport(new Viewport(width, height));
    // this entity renders
    renderer->AddEntity(viewport, order++);

    // Add the camera
    EntityPtr camera(new Camera(m_Joystick));
    // this entity handles events
    m_EventHandlerList.push_back(camera);
    // this entity renders
    renderer->AddEntity(camera, order++);

    // Add a cube
    EntityPtr cube(new Cube);
    // this entity renders
    renderer->AddEntity(cube, order++);

    // Add a cube
    EntityPtr sphere(new Sphere);
    // this entity renders
    renderer->AddEntity(sphere, order++);

    // Run our worker thread
    boost::thread worker(boost::bind(&Worker::Run, m_Worker));

    bool running(true);
    SDL_Event event;
    do
    {
        int eventsPending(0);
        SDL_WaitEvent(&event);
        do
        {
            bool processed(false);
            for ( auto entity = m_EventHandlerList.begin(); entity != m_EventHandlerList.end(); )
            {
                processed |= (*entity)->HandleEvent(event);
                if (processed)
                {
                    break;
                }
                // Remove from event handler as well if marked for deletion
                if ( (*entity)->AreFlagsSet( Entity::F_DELETE ) ) {
                    entity = m_EventHandlerList.erase( entity );
                    continue;
                }
                ++entity;
            }
            if (!processed)
            {
                switch (event.type)
                {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    default:
                        break;
                    }
                    printf("The %s key was pressed!\n",
                            SDL_GetKeyName(event.key.keysym.sym));
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                }
            }
            if (running)
            {
                eventsPending = SDL_PollEvent(&event);
            }
        } while (eventsPending > 0 && running);
    } while (running);

    m_Worker->Terminate();
    worker.join();

    return r;
}

