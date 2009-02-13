//==============================================================================

#include <algorithm>

#include <hgeresource.h>
#include <hgesprite.h>
#include <hgefont.h>

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <viewport.hpp>
#include <score.hpp>

//------------------------------------------------------------------------------

namespace
{
    const char * ACTION_NAME[] =
    {
        "",
        "Move",
        "Halt",
        "Identify",
        "Detain",
        "Attack",
        "Shock"
    };
};

//------------------------------------------------------------------------------
Mouse::MouseButton::MouseButton()
    :
    m_state( MOUSE_START ),
    m_action( ACTION_NONE ),
    m_timer(),
    m_last( 0.0f, 0.0f ),
    m_delta( 0.0f, 0.0f ),
    m_total( 0.0f, 0.0f )
{
}

//------------------------------------------------------------------------------
Mouse::MouseButton::~MouseButton()
{
}

//------------------------------------------------------------------------------
void
Mouse::MouseButton::clear()
{
    m_state = MOUSE_START;
    m_action = ACTION_NONE;
    m_timer = 0.0f;
    Engine::hge()->Input_GetMousePos( & m_last.x, & m_last.y );
    m_delta.SetZero();
    m_total.SetZero();
}

//------------------------------------------------------------------------------
void
Mouse::MouseButton::update( float dt, bool state )
{
    m_timer += dt;

    b2Vec2 position( 0.0f, 0.0f );
    Engine::hge()->Input_GetMousePos( & position.x, & position.y );
    m_delta = position - m_last;
    m_total += m_delta;
    m_last = position;

    float offset( m_total.LengthSquared() );

    switch ( m_state )
    {
        case MOUSE_START:
        {
            if ( state )
            {
                m_timer = 0.0f;
                m_state = MOUSE_FIRST_DOWN;
                m_total.SetZero();
            }
            {
                m_action = ACTION_NONE;
            }
            break;
        }
        case MOUSE_FIRST_DOWN:
        {
            if ( ! state )
            {
                m_timer = 0.0f;
                m_state = MOUSE_FIRST_UP;
                m_total.SetZero();
            }
            else if ( m_timer > 0.2f || offset > 9.0f )
            {
                m_action = ACTION_DRAGGING;
                m_state = MOUSE_END;
            }
            break;
        }
        case MOUSE_FIRST_UP:
        {
            if ( state )
            {
                m_timer = 0.0f;
                m_state = MOUSE_SECOND_DOWN;
                m_total.SetZero();
            }
            else if ( m_timer > 0.1f || offset > 1.0f )
            {
                m_action = ACTION_CLICKED;
                m_state = MOUSE_START;
            }
            break;
        }
        case MOUSE_SECOND_DOWN:
        {
            if ( ! state )
            {
                m_timer = 0.0f;
                m_state = MOUSE_SECOND_UP;
                m_total.SetZero();
            }
            if ( m_timer > 0.2f || offset > 9.0f )
            {
                m_action = ACTION_DRAGGING;
                m_state = MOUSE_END;
            }
            break;
        }
        case MOUSE_SECOND_UP:
        {
            m_action = ACTION_DOUBLED;
            m_state = MOUSE_START;
            break;
        }
        case MOUSE_END:
        {
            if ( ! state )
            {
                m_action = ACTION_NONE;
                m_state = MOUSE_START;
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
bool
Mouse::MouseButton::dragging() const
{
    return m_action == ACTION_DRAGGING;
}

//------------------------------------------------------------------------------
bool
Mouse::MouseButton::clicked() const
{
    return m_action == ACTION_CLICKED;
}

//------------------------------------------------------------------------------
bool
Mouse::MouseButton::doubleClicked() const
{
    return m_action == ACTION_DOUBLED;
}

//------------------------------------------------------------------------------
const b2Vec2 &
Mouse::MouseButton::getDelta() const
{
    return m_delta;
}

//==============================================================================
Mouse::Mouse()
    :
    m_left(),
    m_right()
{
}

//------------------------------------------------------------------------------
Mouse::~Mouse()
{
}

//------------------------------------------------------------------------------
void
Mouse::clear()
{
    m_left.clear();
    m_right.clear();
}

//------------------------------------------------------------------------------
void
Mouse::update( float dt )
{
    m_left.update( dt, Engine::hge()->Input_GetKeyState( HGEK_LBUTTON ) ); 
    m_right.update( dt, Engine::hge()->Input_GetKeyState( HGEK_RBUTTON ) ); 
}

//------------------------------------------------------------------------------
const Mouse::MouseButton &
Mouse::getLeft()
{
    return m_left;
}

//------------------------------------------------------------------------------
const Mouse::MouseButton &
Mouse::getRight()
{
    return m_right;
}

//==============================================================================
Game::Game()
    :
    Context(),
    m_gui( 0 ),
    m_zoom( 2 ),
    m_cars(),
    m_trees(),
    m_guys(),
    m_buildings(),
    m_parked(),
    m_picked( 0 ),
    m_team(),
    m_squad(),
    m_actionType( TYPE_MOVE ),
    m_lock_camera( false ),
    m_locked( 0 ),
    m_mouse()
{
}

//------------------------------------------------------------------------------
Game::~Game()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Game::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    vp->offset().x = 1742.0f;
    vp->offset().y = 2349.0f;
    vp->bounds().x = 800.0f;
    vp->bounds().y = 600.0f;

    Entity::resetNextGroupIndex();

    m_zoom = 3;
    m_picked = 0;
    m_actionType = TYPE_MOVE;
    m_lock_camera = false;
    m_locked = 0;

    m_gui = new hgeSprite( 0, 0, 0, 1, 1 );

    std::vector< Entity * > entities;
    std::vector< Entity * >::iterator i;

    entities = Entity::databaseFactory( TYPE_BUILDING );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        m_buildings.push_back( static_cast< Building * >( * i ) );
    }
    entities = Entity::databaseFactory( TYPE_TREE );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        m_trees.push_back( static_cast< Tree * >( * i ) );
    }
    entities = Entity::databaseFactory( TYPE_PARKED );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        m_parked.push_back( static_cast< Parked * >( * i ) );
    }
    entities = Entity::databaseFactory( TYPE_CAR );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        m_cars.push_back( static_cast< Car * >( * i ) );
    }
    entities = Entity::databaseFactory( TYPE_GUY );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        m_guys.push_back( static_cast< Guy * >( * i ) );
        if ( m_guys.back()->getAllegiance() == ALLEGIANCE_ASSET )
        {
            m_squad.push_back( m_guys.back() );
        }
    }

    m_team.push_back( m_squad.front() );

    b2Vec2 offset( 100.0f, 100.0f );
    b2Vec2 position( m_team.back()->getBody()->GetPosition() );
    b2Shape * shapes[99];
    b2AABB aabb;
    aabb.lowerBound = position - offset;
    aabb.upperBound = position + offset;
    int num( Engine::b2d()->Query( aabb, shapes, 99 ) );
    for ( int i = 0; i < num; ++i )
    {
        Entity * entity =
            static_cast< Entity * >( shapes[0]->GetBody()->GetUserData() );
        if ( entity->getType() == TYPE_BUILDING )
        {
            m_picked = entity;
            break;
        }
    }
    _setViewport( m_picked );

    m_mouse.clear();

    HMUSIC music = Engine::rm()->GetMusic( "game" );
    Engine::hge()->Music_Play( music, true, 50, 0, 0 );
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    while ( m_cars.size() > 0 )
    {
        delete m_cars.back();
        m_cars.pop_back();
    }
    while ( m_trees.size() > 0 )
    {
        delete m_trees.back();
        m_trees.pop_back();
    }
    while ( m_guys.size() > 0 )
    {
        delete m_guys.back();
        m_guys.pop_back();
    }
    while ( m_buildings.size() > 0 )
    {
        delete m_buildings.back();
        m_buildings.pop_back();
    }
    while ( m_parked.size() > 0 )
    {
        delete m_parked.back();
        m_parked.pop_back();
    }

    m_team.clear();
    m_squad.clear();

    Engine::hge()->Channel_StopAll();
    delete m_gui;
    m_gui = 0;
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    ViewPort * vp( Engine::vp() );

    if ( hge->Input_GetKeyState( HGEK_ESCAPE ) &&
         Engine::instance()->isPaused() )
    {
        Engine::instance()->switchContext( STATE_MENU );
        return false;
    }

        /*
        Engine::instance()->switchContext( STATE_SCORE );
        Score * score( static_cast< Score * >(
            Engine::instance()->getContext() ) );
        score->calculateScore( 0, 0, 0, 0 );
        return false;
        */

    _updateCars( dt );
    _updateGuys( dt );
    _updateBuildings( dt );

    m_mouse.update( dt );

    if ( m_mouse.getLeft().doubleClicked() )
    {
        m_zoom += 1;
        if ( m_zoom > 3 )
        {
            m_zoom = 3;
        }
        else
        {
            b2Vec2 point( 0.0f, 0.0f );
            hge->Input_GetMousePos( & point.x, & point.y );
            vp->offset().x += ( 400.0f - point.x ) / vp->hscale();
            vp->offset().y += ( 300.0f - point.y ) / vp->hscale();
            hge->Input_SetMousePos( 400.0f, 300.0f );
        }
    }
    if ( m_mouse.getRight().doubleClicked() )
    {
        m_zoom -= 1;
        if ( m_zoom < 1 )
        {
            m_zoom = 1;
        }
        else
        {
            b2Vec2 point( 0.0f, 0.0f );
            hge->Input_GetMousePos( & point.x, & point.y );
            vp->offset().x += ( 400.0f - point.x ) / vp->hscale();
            vp->offset().y += ( 300.0f - point.y ) / vp->hscale();
            hge->Input_SetMousePos( 400.0f, 300.0f );
        }
    }
    switch ( m_zoom )
    {
        case 1:
        {
            vp->bounds().x = 1600.0f;
            vp->bounds().y = 1200.0f;
            break;
        }
        case 2:
        {
            vp->bounds().x = 800.0f;
            vp->bounds().y = 600.0f;
            break;
        }
        case 3:
        {
            vp->bounds().x = 400.0f;
            vp->bounds().y = 300.0f;
            break;
        }
    }
    int actionDelta( hge->Input_GetMouseWheel() );
    if ( hge->Input_KeyDown( HGEK_UP ) ||
         hge->Input_KeyDown( HGEK_RIGHT ) )
    {
        actionDelta += 1;
    }
    if ( hge->Input_KeyDown( HGEK_DOWN ) ||
         hge->Input_KeyDown( HGEK_LEFT ) )
    {
        actionDelta -= 1;
    }
    if ( actionDelta != 0 )
    {
        int actionType( m_actionType + actionDelta );
        if ( actionType > 6 )
        {
            actionType = 6;
        }
        else if ( actionType < 1 )
        {
            actionType = 1;
        }
        m_actionType = static_cast< ActionType >( actionType );
    }
    if ( hge->Input_KeyDown( HGEK_M ) )
    {
        m_actionType = TYPE_MOVE;
    }
    if ( hge->Input_KeyDown( HGEK_I ) )
    {
        m_actionType = TYPE_INVESTIGATE;
    }
    if ( hge->Input_KeyDown( HGEK_D ) )
    {
        m_actionType = TYPE_RESCUE;
    }
    if ( hge->Input_KeyDown( HGEK_A ) )
    {
        m_actionType = TYPE_ATTACK;
    }
    if ( hge->Input_KeyDown( HGEK_S ) )
    {
        m_actionType = TYPE_AIRSTRIKE;
    }
    if ( m_mouse.getLeft().clicked() )
    {
        b2Vec2 point( 0.0f, 0.0f );
        hge->Input_GetMousePos( & point.x, & point.y );
        vp->screenToWorld( point );
        b2Shape * shapes[9];
        b2AABB aabb;
        b2Vec2 epsilon( 0.1f, 0.1f );
        aabb.lowerBound = point - epsilon;
        aabb.upperBound = point + epsilon;
        int num( Engine::b2d()->Query( aabb, shapes, 9 ) );
        for ( int i = 0; i < num; ++i )
        {   
            Entity * picked = static_cast< Entity * >(
                                shapes[i]->GetBody()->GetUserData() );
            if ( ! picked->getVisible() )
            {
                continue;
            }
            switch ( picked->getType() )
            {
                case TYPE_GUY:
                {
                    Guy * guy( static_cast< Guy * >( picked ) );
                    if ( guy->getAllegiance() == ALLEGIANCE_ASSET )
                    {
                        if ( ! hge->Input_GetKeyState( HGEK_CTRL ) )
                        {
                            m_team.clear();
                        }
                        if ( std::find( m_team.begin(), m_team.end(), guy ) ==
                             m_team.end() )
                        {
                            m_team.push_back( guy );
                        }
                        break;
                    }
                }
                case TYPE_CAR:
                {
                    m_picked = picked;
                    break;
                }
                case TYPE_BUILDING:
                {
                    Building * building( static_cast< Building * >( picked ) );
                    m_picked = static_cast< Entity * >(
                        building->getMeta()->getOwner() );
                    break;
                }
            }
            break;
        }
    }
    if ( hge->Input_KeyDown( HGEK_MBUTTON ) ||
         hge->Input_KeyDown( HGEK_SPACE ) )
    {
        std::vector< Guy * >::iterator i;
        for ( i = m_team.begin(); i != m_team.end(); ++i )
        {
            Target * target( new Target( m_picked ) );
            ( * i )->addAction( Action::factory( m_actionType, target ) );
        }
    }
    if ( m_mouse.getRight().clicked() )
    {
        b2Vec2 point( 0.0f, 0.0f );
        hge->Input_GetMousePos( & point.x, & point.y );
        vp->screenToWorld( point );
        std::vector< Guy * >::iterator i;
        for ( i = m_team.begin(); i != m_team.end(); ++i )
        {
            Target * target( new Target( point ) );
            ( * i )->addAction( Action::factory( m_actionType, target ) );
        }
    }
    if ( hge->Input_KeyDown( HGEK_TAB ) && m_team.size() < m_squad.size() )
    {
        std::vector< Guy * >::iterator i( m_team.begin() );
        std::vector< Guy * >::iterator j( i );
        do
        {
            if ( i != m_team.end() )
            {
                j = find( m_squad.begin(), m_squad.end(), * i );
                ++j;
            }
            else
            {
                j = m_squad.begin();
            }
            if ( j == m_squad.end() )
            {
                j = m_squad.begin();
            }
            if ( i != m_team.end() )
            {
                ++i;
            }
            if ( i == m_team.end() )
            {
                i = m_team.begin();
            }
        }
        while ( find( m_team.begin(), m_team.end(), * j ) != m_team.end() );
        if ( j != m_squad.end() )
        {
            if ( ! hge->Input_GetKeyState( HGEK_CTRL ) )
            {
                m_team.clear();
            }
            if (std::find( m_team.begin(), m_team.end(), * j ) == m_team.end())
            {
                m_team.push_back( * j );
            }
        }
    }
    if ( hge->Input_KeyDown( HGEK_L ) )
    {
        m_lock_camera = ! m_lock_camera;
    }
    if ( hge->Input_KeyDown( HGEK_1 ) && m_squad.size() > 0 )
    {
        _setViewport( m_squad[0] );
    }
    if ( hge->Input_KeyDown( HGEK_2 ) && m_squad.size() > 1 )
    {
        _setViewport( m_squad[1] );
    }
    if ( hge->Input_KeyDown( HGEK_3 ) && m_squad.size() > 2 )
    {
        _setViewport( m_squad[2] );
    }
    if ( hge->Input_KeyDown( HGEK_4 ) && m_squad.size() > 3 )
    {
        _setViewport( m_squad[3] );
    }
    if ( hge->Input_KeyDown( HGEK_5 ) )
    {
        _setViewport( m_picked );
    }
    if ( hge->Input_KeyDown( HGEK_1 ) )
    {
        _setViewport( m_squad[0] );
    }
    if ( m_mouse.getLeft().dragging() )
    {
        vp->offset().x += m_mouse.getLeft().getDelta().x / vp->hscale();
        vp->offset().y += m_mouse.getLeft().getDelta().y / vp->vscale();
    }
    else if ( m_mouse.getRight().dragging() )
    {
        vp->offset().x += m_mouse.getRight().getDelta().x / vp->hscale();
        vp->offset().y += m_mouse.getRight().getDelta().y / vp->vscale();
    }

    if ( m_locked != 0 && m_lock_camera )
    {
        b2Vec2 position( m_locked->getBody()->GetPosition() );
        vp->offset() = -1.0f * position;
        vp->offset().x += 0.5f * vp->bounds().x * vp->hscale();
        vp->offset().y += 0.5f * vp->bounds().y * vp->vscale();
    }

    float xmax( 400.0f + 2500.0f - 0.5f * vp->bounds().x );
    float xmin( 400.0f - 2500.0f + 0.5f * vp->bounds().x );
    float ymax( 300.0f + 2500.0f - 0.5f * vp->bounds().y );
    float ymin( 300.0f - 2500.0f + 0.5f * vp->bounds().y );

    xmax += 0.5f / vp->hscale();
    ymax += 0.5f / vp->vscale();
    if ( vp->offset().x > xmax )
    {
        vp->offset().x = xmax;
    }
    if ( vp->offset().x < xmin )
    {
        vp->offset().x = xmin;
    }
    if ( vp->offset().y > ymax )
    {
        vp->offset().y = ymax;
    }
    if ( vp->offset().y < ymin )
    {
        vp->offset().y = ymin;
    }

    return false;
}

//------------------------------------------------------------------------------
void
Game::render()
{
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );
    hgeResourceManager * rm( Engine::rm() );

    hge->Gfx_SetTransform( 400.0f,
                           300.0f,
                           vp->offset().x * vp->hscale(),
                           vp->offset().y * vp->vscale(),
                           vp->angle(),
                           vp->hscale(),
                           vp->vscale() );

    rm->GetSprite( "map11" )->Render( -2000, -2000 );
    rm->GetSprite( "map21" )->Render( -1000, -2000 );
    rm->GetSprite( "map31" )->Render( 0, -2000 );
    rm->GetSprite( "map41" )->Render( 1000, -2000 );
    rm->GetSprite( "map51" )->Render( 2000, -2000 );
    rm->GetSprite( "map12" )->Render( -2000, -1000 );
    rm->GetSprite( "map22" )->Render( -1000, -1000 );
    rm->GetSprite( "map32" )->Render( 0, -1000 );
    rm->GetSprite( "map42" )->Render( 1000, -1000 );
    rm->GetSprite( "map52" )->Render( 2000, -1000 );
    rm->GetSprite( "map13" )->Render( -2000, 0 );
    rm->GetSprite( "map23" )->Render( -1000, 0 );
    rm->GetSprite( "map33" )->Render( 0, 0 );
    rm->GetSprite( "map43" )->Render( 1000, 0 );
    rm->GetSprite( "map53" )->Render( 2000, 0 );
    rm->GetSprite( "map14" )->Render( -2000, 1000 );
    rm->GetSprite( "map24" )->Render( -1000, 1000 );
    rm->GetSprite( "map34" )->Render( 0, 1000 );
    rm->GetSprite( "map44" )->Render( 1000, 1000 );
    rm->GetSprite( "map54" )->Render( 2000, 1000 );
    rm->GetSprite( "map15" )->Render( -2000, 2000 );
    rm->GetSprite( "map25" )->Render( -1000, 2000 );
    rm->GetSprite( "map35" )->Render( 0, 2000 );
    rm->GetSprite( "map45" )->Render( 1000, 2000 );
    rm->GetSprite( "map55" )->Render( 2000, 2000 );

    _renderBodies();        
    Engine::pm()->Render();

    rm->GetSprite( "shadow11" )->RenderEx( -1250, -1250, 0.0f, 2.5f, 2.5f );
    rm->GetSprite( "shadow21" )->RenderEx( 1250, -1250, 0.0f, 2.5f, 2.5f );
    rm->GetSprite( "shadow12" )->RenderEx( -1250, 1250, 0.0f, 2.5f, 2.5f );
    rm->GetSprite( "shadow22" )->RenderEx( 1250, 1250, 0.0f, 2.5f, 2.5f );

    _renderGuis();        

    float width( 0.5f / vp->hscale() );
    m_gui->SetColor( 0x55AAFF88 );
    for ( float i = -2500.0f; i <= 2500.0f; i += 200.0f )
    {
        m_gui->RenderStretch( i - width, -2500.0f, i + width, 2500.0f );
        m_gui->RenderStretch( -2500.0f, i - width, 2500.0f, i + width );
    }
    m_gui->SetColor( 0x88FFFF88 );
    for ( float i = -2500.0f; i <= 2500.0f; i += 1000.0f )
    {
        m_gui->RenderStretch( i - width, -2500.0f, i + width, 2500.0f );
        m_gui->RenderStretch( -2500.0f, i - width, 2500.0f, i + width );
    }

    if ( m_picked != 0 )
    {
        const b2AABB & aabb( m_picked->getAABB() );
        _renderTarget( m_picked->getColor(), aabb );
    }
    std::vector< Guy * >::iterator i;
    for ( i = m_team.begin(); i != m_team.end(); ++i )
    {
        const b2AABB & aabb( ( * i )->getAABB() );
        _renderTarget( ( * i )->getColor(), aabb );
    }

    hge->Gfx_SetTransform();
    _renderGui();
}

//------------------------------------------------------------------------------
// private
//------------------------------------------------------------------------------
void
Game::_updateCars( float dt )
{
    std::vector< Car * >::iterator i;
    for ( i = m_cars.begin(); i != m_cars.end();  ++i )
    {
        ( * i )->update( dt );
        if ( ( * i )->isDestroyed() )
        {
            // TODO: eject occupants, explode, replace with charred body
        }
    }
}

//------------------------------------------------------------------------------
void
Game::_updateGuys( float dt )
{
    std::vector< Guy * >::iterator i;
    for ( i = m_guys.begin(); i != m_guys.end(); ++i )
    {
        ( * i )->update( dt );
        if ( ( * i )->isDestroyed() )
        {
            // TODO: replace with crucifix
        }
    }
}

//------------------------------------------------------------------------------
void
Game::_updateBuildings( float dt )
{
    std::vector< Building * >::iterator i;
    for ( i = m_buildings.begin(); i != m_buildings.end(); ++i )
    {
        ( * i )->update( dt );
        if ( ( * i )->getMeta()->isDestroyed() &&
             ( * i )->getMeta()->getOwner() == ( * i ) )
        {
            // TODO: eject occupants
        }
    }
}

//------------------------------------------------------------------------------
void
Game::_renderBodies()
{
    ViewPort * vp( Engine::vp() );
    b2World * b2d( Engine::b2d() );

    for ( b2Body * body( b2d->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        if ( body->IsDynamic() )
        {
            Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
            if ( entity )
            {
                entity->render();
            }   
        }
    }   
}   

//------------------------------------------------------------------------------
void
Game::_renderGuis()
{
    ViewPort * vp( Engine::vp() );
    b2World * b2d( Engine::b2d() );

    for ( b2Body * body( b2d->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
        if ( entity )
        {
            switch ( entity->getType() )
            {
                case TYPE_CAR:
                case TYPE_BUILDING:
                case TYPE_GUY:
                {
                    entity->renderGui( m_gui, m_zoom, m_picked );
                }
            }
        }   
    }   
}   

//------------------------------------------------------------------------------
void
Game::_renderTarget( DWORD color, const b2AABB & aabb )
{
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );
    float width( 7.0f / vp->hscale() );
    float height( 7.0f / vp->vscale() );
    float dx( 2.0f / vp->hscale() );
    float dy( 2.0f / vp->vscale() );
    m_gui->SetColor( color );
    m_gui->RenderStretch( aabb.lowerBound.x - width, aabb.lowerBound.y - height,
                          aabb.lowerBound.x, aabb.lowerBound.y - height + dy );
    m_gui->RenderStretch( aabb.lowerBound.x - width, aabb.lowerBound.y - height,
                          aabb.lowerBound.x - width + dx, aabb.lowerBound.y );
    m_gui->RenderStretch( aabb.upperBound.x + width, aabb.lowerBound.y - height,
                          aabb.upperBound.x, aabb.lowerBound.y - height + dy );
    m_gui->RenderStretch( aabb.upperBound.x + width, aabb.lowerBound.y - height,
                          aabb.upperBound.x + width - dx, aabb.lowerBound.y );
    m_gui->RenderStretch( aabb.upperBound.x + width, aabb.upperBound.y + height,
                          aabb.upperBound.x, aabb.upperBound.y + height - dy );
    m_gui->RenderStretch( aabb.upperBound.x + width, aabb.upperBound.y + height,
                          aabb.upperBound.x + width - dx, aabb.upperBound.y );
    m_gui->RenderStretch( aabb.lowerBound.x - width, aabb.upperBound.y + height,
                          aabb.lowerBound.x, aabb.upperBound.y + height - dy );
    m_gui->RenderStretch( aabb.lowerBound.x - width, aabb.upperBound.y + height,
                          aabb.lowerBound.x - width + dx, aabb.upperBound.y );
}

//------------------------------------------------------------------------------
void
Game::_renderGui()
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );
    float width =
        static_cast< float >( hge->System_GetState( HGE_SCREENWIDTH ) );
    float height =
        static_cast< float >( hge->System_GetState( HGE_SCREENHEIGHT ) );
    hgeFont * font( rm->GetFont( "dialogue" ) );
    font->SetColor( 0xFFFFCCFF );
    ViewPort * vp( Engine::vp() );
    b2Vec2 mouse( 0.0f, 0.0f );
    hge->Input_GetMousePos( & mouse.x, & mouse.y );
    b2Vec2 point( mouse );
    vp->screenToWorld( point );

    if ( m_lock_camera )
    {
        font->printf( 10.0f, 10.0f, HGETEXT_LEFT, "CAMERA LOCKED" );
    }

    m_gui->SetColor( 0x88000000 );
    m_gui->RenderStretch( 540.0f, 6.0f, 740.0f, 25.0f );

    font->printf( 640.0f, 10.0f, HGETEXT_CENTER, "%s -> %s %s",
                  ACTION_NAME[m_actionType],
                  m_picked->getAllegianceName(),
                  m_picked->getTypeName() );

    m_gui->RenderStretch( 300.0f, 776.0F, 980.0f, 795.0f );
    for ( unsigned int i = 0; i < m_squad.size(); ++i )
    {
        if (std::find( m_team.begin(),m_team.end(),m_squad[i]) != m_team.end() )
        {
            font->printf( 390.0f + 160.0f * i, 780.0f, HGETEXT_CENTER,
                          "[%s]", m_squad[i]->getName() );
        }
        else
        {
            font->printf( 390.0f + 160.0f * i, 780.0f, HGETEXT_CENTER,
                          m_squad[i]->getName() );
        }
        font->printf( 310.0f + 160.0f * i, 780.0f, HGETEXT_CENTER, "|" );
    }
    font->printf( 960.0f, 780.0f, HGETEXT_CENTER, "|" );

    hge->Gfx_RenderLine( 0.0f, mouse.y, mouse.x - 5.0f, mouse.y, 0xCCFFFFFF );
    hge->Gfx_RenderLine( mouse.x + 5.0f, mouse.y, 800.0f, mouse.y, 0xCCFFFFFF );
    hge->Gfx_RenderLine( mouse.x, 0.0f, mouse.x, mouse.y - 5.0f, 0xCCFFFFFF );
    hge->Gfx_RenderLine( mouse.x, mouse.y + 5.0f, mouse.x, 600.0f, 0xCCFFFFFF );
    hge->Gfx_RenderLine( mouse.x - 6.0f, mouse.y - 10.0f,
                         mouse.x + 6.0f, mouse.y - 10.0f, 0xEEFFFFFF ); 
    hge->Gfx_RenderLine( mouse.x + 10.0f, mouse.y - 6.0f,
                         mouse.x + 10.0f, mouse.y + 6.0f, 0xEEFFFFFF ); 
    hge->Gfx_RenderLine( mouse.x + 6.0f, mouse.y + 10.0f,
                         mouse.x - 6.0f, mouse.y + 10.0f, 0xEEFFFFFF ); 
    hge->Gfx_RenderLine( mouse.x - 10.0f, mouse.y + 6.0f,
                         mouse.x - 10.0f, mouse.y - 6.0f, 0xEEFFFFFF ); 
}

//------------------------------------------------------------------------------
void
Game::_setViewport( Entity * entity )
{
    if ( entity == 0 || entity->getBody() == 0 )
    {
        return;
    }
    m_locked = entity;
    ViewPort * vp( Engine::vp() );
    b2Vec2 position( entity->getBody()->GetPosition() );
    vp->offset() = -1.0f * position;
    vp->offset().x += 0.5f * vp->bounds().x * vp->hscale();
    vp->offset().y += 0.5f * vp->bounds().y * vp->vscale();
}

//==============================================================================
