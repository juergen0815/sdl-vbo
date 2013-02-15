/*
 * cube.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef CUBE_H_
#define CUBE_H_

#include "err.h"
#include "entity.h"
#include "vector.h"

#include <GL/glew.h>

class Cube : public Entity
{
	GLuint m_VboID;

    Vector m_Position;
    Vector m_Scale;
    Vector m_Rotation;
public:
	Cube();

	virtual ~Cube();
private:
	virtual bool Initialize();

	virtual bool HandleEvent( const SDL_Event& event );

	virtual void Render( long ticks );

};

#endif /* CUBE_H_ */
