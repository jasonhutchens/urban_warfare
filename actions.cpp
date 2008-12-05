//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>

#include <engine.hpp>
#include <viewport.hpp>
#include <entity.hpp>
#include <actions.hpp>

//==============================================================================
ActionTaker::ActionTaker()
    :
    m_supported( TYPE_NONE ),
    m_entity( 0 ),
    m_actions()
{
}

//------------------------------------------------------------------------------
ActionTaker::~ActionTaker()
{
    std::map< ActionType, Action * >::iterator i;
    for ( i = m_actions.begin(); i != m_actions.end(); )
    {
        delete i->second;
        m_actions.erase( i++ );
    }
}

//------------------------------------------------------------------------------
bool
ActionTaker::hasAction( ActionType type )
{
    return m_actions.find( type ) != m_actions.end();
}

//------------------------------------------------------------------------------
Action *
ActionTaker::getAction( ActionType type )
{
    std::map< ActionType, Action * >::iterator i( m_actions.find( type ) );
    if ( i == m_actions.end() )
    {
        return 0;
    }
    return i->second;
}

//------------------------------------------------------------------------------
void
ActionTaker::addAction( Action * action )
{
    if ( ( action->getType() & m_supported ) == 0 )
    {
        Engine::hge()->System_Log( "Action not supported" );
        return;
    }
    Action * oldAction( getAction( action->getType() ) );
    if ( oldAction != 0 )
    {
        Engine::hge()->System_Log( "Stopping old action" );
        stopAction( oldAction );
    }
    action->setEntity( m_entity );
    m_actions.insert( std::pair< ActionType, Action * >( action->getType(),
                                                         action ) );
    action->init();
}

//------------------------------------------------------------------------------
void
ActionTaker::stopAction( Action * action )
{
    ActionType type( action->getType() );
    std::map< ActionType, Action * >::iterator i( m_actions.find( type ) );
    if ( i == m_actions.end() )
    {
        return;
    }
    i->second->getEntity()->onActionInterrupted( i->second );
    delete i->second;
    m_actions.erase( i );
}

//------------------------------------------------------------------------------
//protected::
//------------------------------------------------------------------------------
void
ActionTaker::collideActions( Entity * entity, b2ContactPoint * point )
{
    std::map< ActionType, Action * >::iterator i;
    for ( i = m_actions.begin(); i != m_actions.end(); ++i )
    {
        i->second->collide( entity, point );
    }
}

//------------------------------------------------------------------------------
void
ActionTaker::updateActions( float dt )
{
    std::map< ActionType, Action * >::iterator i;
    for ( i = m_actions.begin(); i != m_actions.end(); )
    {
        Action * action( i->second );
        action->update( dt );
        if ( ! action->isComplete() )
        {
            ++i;
            continue;
        }
        action->getEntity()->onActionCompleted( action );
        delete action;
        m_actions.erase( i++ );
    }
}

//------------------------------------------------------------------------------
void
ActionTaker::renderActions()
{
    std::map< ActionType, Action * >::iterator i;
    for ( i = m_actions.begin(); i != m_actions.end(); ++i )
    {
        i->second->render();
    }
}

//==============================================================================
Action::Action( Target * target )
    :
    m_entity( 0 ),
    m_target( target ),
    m_type( TYPE_NONE ),
    m_complete( false )
{
}

//------------------------------------------------------------------------------
Action::~Action()
{
    delete m_target;
}

//------------------------------------------------------------------------------
bool
Action::isComplete()
{
    return m_complete;
}

//------------------------------------------------------------------------------
ActionType
Action::getType()
{
    return m_type;
}

//------------------------------------------------------------------------------
Target *
Action::getTarget()
{
    return m_target;
}

//------------------------------------------------------------------------------
//static:
//------------------------------------------------------------------------------
Action *
Action::factory( ActionType type, Target * target )
{
    switch ( type )
    {
        case TYPE_MOVE:
        {
            return new MoveAction( target );
        }
        case TYPE_HALT:
        {
            return new HaltAction( target );
        }
        case TYPE_INVESTIGATE:
        {
            return new InfoAction( target );
        }
        case TYPE_RESCUE:
        {
            return new RescueAction( target );
        }
        case TYPE_ATTACK:
        {
            return new AttackAction( target );
        }
        case TYPE_AIRSTRIKE:
        {
            return new StrikeAction( target );
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Action::setEntity( Entity * entity )
{
    m_entity = entity;
}

//------------------------------------------------------------------------------
Entity *
Action::getEntity()
{
    return m_entity;
}

//==============================================================================
Target::Target( Entity * entity )
    :
    m_entity( entity ),
    m_position( 0.0f, 0.0f )
{
}

//------------------------------------------------------------------------------
Target::Target( b2Vec2 position )
    :
    m_entity( 0 ),
    m_position( position )
{
}

//------------------------------------------------------------------------------
Target::~Target()
{
}

//------------------------------------------------------------------------------
const b2Vec2 &
Target::getPosition()
{
    if ( m_entity != 0 )
    {
        return m_entity->getBody()->GetPosition();
    }
    return m_position;
}

//------------------------------------------------------------------------------
Entity *
Target::getEntity()
{
    return m_entity;
}

//==============================================================================
MoveAction::MoveAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_MOVE;
}

//------------------------------------------------------------------------------
MoveAction::~MoveAction()
{
}

//------------------------------------------------------------------------------
void
MoveAction::init()
{
}

//------------------------------------------------------------------------------
void
MoveAction::collide( Entity * entity, b2ContactPoint * point )
{
    if ( m_target->getEntity() == 0 ||
         entity->getType() != m_target->getEntity()->getType() )
    {
        return;
    }
    if ( entity->getType() == TYPE_BUILDING )
    {
        if ( static_cast< Building * >( entity )->getMeta() ==
             static_cast< Building * >( m_target->getEntity() )->getMeta() )
        {
            _completeAction();
        }
    }
    else if ( entity->getType() == TYPE_CAR )
    {
        if ( entity == m_target->getEntity() )
        {
            _completeAction();
        }
    }
    else if ( entity == m_target->getEntity() )
    {
        _completeAction();
    }
}

//------------------------------------------------------------------------------
void
MoveAction::update( float dt )
{
    m_entity->getBody()->WakeUp();

    b2Vec2 position( m_entity->getBody()->GetPosition() );
    b2Vec2 direction( m_target->getPosition() - position );
    float length( direction.Normalize() );
    b2Vec2 vertical( 0.0f, -1.0f );
    b2Mat22 rotation( m_entity->getBody()->GetAngle() );
    b2Vec2 heading( b2Mul( rotation, vertical ) );

    if ( length < 2.0f )
    {
        _completeAction();
        return;
    }

    // Perturb this to avoid collisions with obstacles

    /*
    b2AABB aabb;
    b2Vec2 range( 100.0f, 100.0f );
    aabb.lowerBound = m_entity->getBody()->GetPosition() - range;
    aabb.upperBound = m_entity->getBody()->GetPosition() + range;
    b2Shape * shapes[100];
    int num( Engine::b2d()->Query( aabb, shapes, 100 ) );
    bool done( false );
    for ( int i = 0; i < num; ++ i )
    {
        b2Shape * shape( shapes[i] );
        Entity * entity( static_cast< Entity * >(
            shape->GetBody()->GetUserData() ) );
        if ( entity == m_entity )
        {
            continue;
        }
        if ( m_target->getEntity() == entity )
        {
            done = true;
        }
    }
    */

    float magnitude( 0.5f * ( 1.0f - b2Dot( heading, direction ) ) );
    m_entity->getBody()->SetLinearVelocity(
        10.0f * ( 1.0f - magnitude ) * heading );
    m_entity->getBody()->SetAngularVelocity(
        10.0f * magnitude * b2Cross( heading, direction ) );
}

//------------------------------------------------------------------------------
void
MoveAction::render()
{
    return;
    b2Vec2 position( m_entity->getBody()->GetPosition() );
    Engine::hge()->Gfx_RenderLine( position.x, position.y,
                                   m_target->getPosition().x,
                                   m_target->getPosition().y, 0xAAFFFF55 );
    hgeFont * font( Engine::rm()->GetFont( "dialogue" ) );
    font->SetScale( 1.0f / Engine::vp()->hscale() );
    b2Body * body( m_entity->getBody() );
    if ( isComplete() )
    {
        font->printf( body->GetPosition().x, body->GetPosition().y,
                      HGETEXT_CENTER, "[complete]" );
    }
    else
    {
        font->printf( body->GetPosition().x, body->GetPosition().y + 15.0f,
                      HGETEXT_CENTER, "[moving]" );
    }
    font->SetScale( 1.0f );
}

//------------------------------------------------------------------------------
//private:
//------------------------------------------------------------------------------
void
MoveAction::_completeAction()
{
    m_complete = true;
    m_entity->getBody()->SetAngularVelocity( 0.0f );
    b2Vec2 zero( 0.0f, 0.0f );
    m_entity->getBody()->SetLinearVelocity( zero );
}

//==============================================================================
HaltAction::HaltAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_HALT;
}

//------------------------------------------------------------------------------
HaltAction::~HaltAction()
{
}

//------------------------------------------------------------------------------
void
HaltAction::init()
{
}

//------------------------------------------------------------------------------
void
HaltAction::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
HaltAction::update( float dt )
{
    m_complete = true;
    m_entity->getBody()->SetAngularVelocity( 0.0f );
    b2Vec2 zero( 0.0f, 0.0f );
    m_entity->getBody()->SetLinearVelocity( zero );
}

//------------------------------------------------------------------------------
void
HaltAction::render()
{
}

//==============================================================================
InfoAction::InfoAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_INVESTIGATE;
}

//------------------------------------------------------------------------------
InfoAction::~InfoAction()
{
}

//------------------------------------------------------------------------------
void
InfoAction::init()
{
}

//------------------------------------------------------------------------------
void
InfoAction::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
InfoAction::update( float dt )
{
}

//------------------------------------------------------------------------------
void
InfoAction::render()
{
}

//==============================================================================
RescueAction::RescueAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_RESCUE;
}

//------------------------------------------------------------------------------
RescueAction::~RescueAction()
{
}

//------------------------------------------------------------------------------
void
RescueAction::init()
{
}

//------------------------------------------------------------------------------
void
RescueAction::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
RescueAction::update( float dt )
{
}

//------------------------------------------------------------------------------
void
RescueAction::render()
{
}

//==============================================================================
AttackAction::AttackAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_ATTACK;
}

//------------------------------------------------------------------------------
AttackAction::~AttackAction()
{
}

//------------------------------------------------------------------------------
void
AttackAction::init()
{
}

//------------------------------------------------------------------------------
void
AttackAction::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
AttackAction::update( float dt )
{
}

//------------------------------------------------------------------------------
void
AttackAction::render()
{
}

//==============================================================================
StrikeAction::StrikeAction( Target * target )
    :
    Action( target )
{
    m_type = TYPE_AIRSTRIKE;
}

//------------------------------------------------------------------------------
StrikeAction::~StrikeAction()
{
}

//------------------------------------------------------------------------------
void
StrikeAction::init()
{
}

//------------------------------------------------------------------------------
void
StrikeAction::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
StrikeAction::update( float dt )
{
}

//------------------------------------------------------------------------------
void
StrikeAction::render()
{
}

//==============================================================================
