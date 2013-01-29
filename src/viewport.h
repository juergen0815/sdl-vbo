/*
 * viewport.h
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include "err.h"
#include "entity.h"

class Viewport : public Entity
{
    int    m_Width;
    int    m_Height;
public:
    Viewport( int width, int height );

    virtual ~Viewport();

private:
    virtual bool Initialize();

    virtual bool HandleEvent( const SDL_Event& event );

    virtual void Render( long ticks );

    virtual void PostRender( );
};


#endif /* VIEWPORT_H_ */
