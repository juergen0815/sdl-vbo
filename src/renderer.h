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



class Renderer : public Worker
{
	bool m_Terminate;

    EntityList m_InitList;
	EntityList m_RenderList;
	EntityList m_DestroyList;

	HGLRC m_CurrentContext;
	HDC   m_CurrentDC;

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
