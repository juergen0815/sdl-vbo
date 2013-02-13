/*
 * entity.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include <SDL/SDL_events.h>

#include <boost/shared_ptr.hpp>

#include <list>

class Entity
{
    enum enFLAG_BITS {
        F_ENABLE_B = 0,
        F_VISIBLE_B,
        F_DELETE_B,
    };
public:
    enum enFLAG
    {
        F_ENABLE  = (1<<F_ENABLE_B),
        F_VISIBLE = (1<<F_VISIBLE_B),
        F_DELETE  = (1<<F_DELETE_B),
    };
private:
    uint32_t m_Flags;
    int      m_OrderNum;
public:
    Entity() : m_Flags(F_ENABLE), m_OrderNum(0) {}

	virtual ~Entity() {}

	virtual bool HandleEvent( const SDL_Event& event ) = 0;

	uint32_t GetFlags() const { return m_Flags; }

	bool AreFlagsSet( enFLAG flags ) const { return (m_Flags & flags) == flags; }

	// Only renderer has access to these below
private:
    void SetOrder( int order ) { m_OrderNum = order; }

    int GetOrder() const { return m_OrderNum; };

    void SetFlag( enFLAG flag ) { m_Flags |= flag; }

    void ClearFlag( enFLAG flag ) { m_Flags &= ~flag; }

protected:
    virtual bool Initialize() = 0;

	virtual void Render( long ticks ) = 0;

	friend class Renderer;
};

typedef boost::shared_ptr< Entity > EntityPtr;

typedef std::list< EntityPtr > EntityList;

#endif /* ENTITY_H_ */
