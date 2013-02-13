/*
 * cube.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef CUBE_H_
#define CUBE_H_

#include "entity.h"

#include <GL/glew.h>

class Cube : public Entity
{
	GLuint m_VboID;
public:
	Cube();

	virtual ~Cube();
private:
	virtual bool Initialize();

	virtual bool HandleEvent( const SDL_Event& event );

	virtual void Render( long ticks );

    virtual void PostRender( );

};


#endif /* CUBE_H_ */
