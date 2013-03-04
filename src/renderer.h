/*
 * render.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef RENDER_H_
#define RENDER_H_

#include "worker.h"
#include "entity.h"

#include <list>

#include <boost/shared_ptr.hpp>
#include <GL/glew.h>
#ifdef __linux__
#include <GL/glx.h>
#endif
#include <SDL/SDL_syswm.h>

class Renderer : public Worker
{
	bool m_Terminate;

    EntityList m_InitList;
	EntityList m_RenderList;
	EntityList m_DestroyList;

#ifdef _WIN32
	HGLRC       m_CurrentContext;
	HDC         m_CurrentDC;
#endif
#ifdef __linux__
	GLXContext   m_CurrentContext;
#endif

public:
	Renderer();

	virtual ~Renderer();

	void Init();

	void AddEntity( EntityPtr entity, int priority = 0 );

	void RemoveEntity( EntityPtr entity );
private:
	void InitGL();

	// No direct access
	virtual void Terminate();

	virtual void Run();

private:
	// Need this in scope of class to have access to Entity::GetPriority() method
	bool CompareEntityPriorities( const EntityPtr& a, const EntityPtr& b );

};

#endif /* RENDER_H_ */
