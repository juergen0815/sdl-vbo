
/*
 * viewport.cpp
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#include "viewport.h"

#include <GL/glew.h>

Viewport::Viewport( int width, int height )
    : m_Width(width)
    , m_Height(height)
{
}

Viewport::~Viewport()
{
}

bool Viewport::Initialize()
{
    return true;
}

bool Viewport::HandleEvent(const SDL_Event& event)
{
    return false;
}

void Viewport::Render(long ticks)
{
    // Only 3D we do not care about overlays - just yet
    glViewport(0, 0, (GLsizei)m_Width, (GLsizei)m_Height);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, ((float)m_Width)/((float)m_Height), 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

}

void Viewport::PostRender()
{
}



