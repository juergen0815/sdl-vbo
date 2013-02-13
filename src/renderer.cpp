/*
 * render.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "renderer.h"
#include "err.h"

#include <SDL/SDL.h>

#include <boost/bind.hpp>

#include <GL/glew.h>

static bool compareEntityPtr( const EntityPtr& a, const EntityPtr& b )
{
	return a.get() == b.get();
}

Renderer::Renderer()
	: m_Terminate(false)
    , m_CurrentContext( nullptr )
    , m_CurrentDC( nullptr )
{
}

Renderer::~Renderer()
{
}

void Renderer::Init()
{
    m_CurrentContext = wglGetCurrentContext();
    m_CurrentDC      = wglGetCurrentDC();

    // release current context
    wglMakeCurrent( NULL, NULL );
}

void Renderer::AddEntity( EntityPtr entity, int priority /*= 0*/  )
{
    entity->SetOrder( priority );
    m_InitList.push_back( entity );
}

void Renderer::RemoveEntity( EntityPtr entity )
{
    entity->SetFlag( Entity::F_DELETE );

//	m_DestroyList.push_back( entity );
}

void Renderer::Terminate()
{
	m_Terminate = true;
}

void Renderer::InitGL()
{
    // This is important! Our renderer runs its own render thread
    // All
    wglMakeCurrent(m_CurrentDC,m_CurrentContext);

    // Init GLEW - we need this to use OGL extensions (e.g. for VBOs)
    GLenum err = glewInit();
    ASSERT( GLEW_OK == err, "Error: %s\n", glewGetErrorString(err) );

    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration

}

bool Renderer::CompareEntityPriorities( const EntityPtr& a, const EntityPtr& b ) {
    return a->GetOrder() < b->GetOrder();
}

static void SendTerminate()
{
    // Send QUIT event to main thread
    SDL_Event quit;
    quit.type = SDL_QUIT;
    SDL_PushEvent( &quit );
}

static void HandleUnexpected()
{
    ShowError( "Renderer failed with unexpected Exception!", "Unexpected Exception in Renderer" );
    exit(-20);
}

void Renderer::Run()
{
    std::set_terminate( SendTerminate );
    std::set_unexpected( HandleUnexpected );
    try {
        InitGL();

        long ticks = SDL_GetTicks();
        do {
            // first step: iterate through a list of newly added entities and initialize them properly
            //             Must be done in the context of the render thread.
            //             Limit the number of initializations to 5 to not stall the render loop

            int resort(0);
            // max init 5 entities at one time to not stall the render loop forever
            for ( int initLimit = 5; (m_InitList.size() > 0) && initLimit > 0; ++resort, --initLimit ) {
                auto& entity = m_InitList.front();
                entity->Initialize();
                m_RenderList.push_back(entity);
                m_InitList.pop_front();
            }

            // second step: If we added more than 1 entity, resort the render list

            // only resort if we added more than one entity
            if ( resort > 0 ) {
                m_RenderList.sort( boost::bind( &Renderer::CompareEntityPriorities, this, _1, _2) );
            }

            // third step: render all entities

            // No Scene graph, no nested objects, no tree...must unroll in reverse order (see below)
            // DONT DO THIS. Just to keep it simple! Use a scene graph instead!

            // run list
            long timeStamp = SDL_GetTicks();
            for( auto& entity : m_RenderList ) {
                if ( entity->AreFlagsSet( Entity::F_ENABLE ) ) {
                    entity->Render( timeStamp - ticks );
                }
            }
            // fourth: swap the buffers
            // Swap the buffer
            SDL_GL_SwapBuffers();
            ticks = timeStamp;

            // remove after we are done with the rendering. Can't remove in first list since this would mess up PostRender
            for( auto entity = m_RenderList.begin(); entity != m_RenderList.end(); ) {
                // does this mess up my iterator?? - maybe not in reverse order
                if ( (*entity)->AreFlagsSet( Entity::F_DELETE ) ) {
                    entity = m_RenderList.erase( entity );
                    continue;
                }
                ++entity;
            }

        } while (!m_Terminate);

        m_RenderList.clear();
    }
    catch ( std::bad_alloc & ex ) {
        ShowError( ex.what(), "Memory Exception in Renderer" );
        m_RenderList.clear();
        m_Terminate = true;
        SendTerminate();
    }
    catch ( std::exception &ex ) {
        ShowError( ex.what(), "Exception in Renderer" );
        m_RenderList.clear();
        m_Terminate = true;
        SendTerminate();
    }
    catch ( ... ) {
        ShowError( "Renderer failed with unknown error!", "Unknown Exception inn Renderer" );
        m_RenderList.clear();
        m_Terminate = true;
        SendTerminate();
    }
}


