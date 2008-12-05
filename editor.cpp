//==============================================================================

#include <algorithm>

#include <hgeresource.h>
#include <hgesprite.h>
#include <hgefont.h>

#include <editor.hpp>
#include <engine.hpp>
#include <viewport.hpp>
#include <debug.hpp>
#include <entity.hpp>

//------------------------------------------------------------------------------
Editor::Editor()
    :
    Context(),
    m_gui( 0 ),
    m_zoom( 2 ),
    m_mouse( 0.0f, 0.0f ),
    m_radius( 15.0f ),
    m_width( 50.0f ),
    m_height( 50.0f ),
    m_angle( 0.0f ),
    m_trees(),
    m_show_map( true ),
    m_mode( MODE_VIEW ),
    m_buildings(),
    m_parked(),
    m_cars(),
    m_guys(),
    m_kind( 0 ),
    m_picked( 0 )
{
}

//------------------------------------------------------------------------------
Editor::~Editor()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Editor::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    vp->offset().x = 1760.0f;
    vp->offset().y = 2380.0f;
    vp->bounds().x = 800.0f;
    vp->bounds().y = 600.0f;

    Entity::resetNextGroupIndex();

    m_zoom = 2;
    m_picked = 0;

    m_gui = new hgeSprite( 0, 0, 0, 1, 1 );
    m_gui->SetColor( 0xAAFFFF55 );

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
    }

    Engine::hge()->Channel_StopAll();
}

//------------------------------------------------------------------------------
void
Editor::fini()
{
    while ( m_trees.size() > 0 )
    {
        delete m_trees.back();
        m_trees.pop_back();
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
    while ( m_cars.size() > 0 )
    {
        delete m_cars.back();
        m_cars.pop_back();
    }
    while ( m_guys.size() > 0 )
    {
        delete m_guys.back();
        m_guys.pop_back();
    }
    delete m_gui;
    m_gui = 0;
}

//------------------------------------------------------------------------------
bool
Editor::update( float dt )
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    ViewPort * vp( Engine::vp() );

    if ( hge->Input_GetKeyState( HGEK_ESCAPE ) )
    {
        Engine::instance()->switchContext( STATE_MENU );
        return false;
    }

    if ( hge->Input_GetMouseWheel() > 0 )
    {
        m_zoom += 1;
        if ( m_zoom > 3 )
        {
            m_zoom = 3;
        }
        hge->Input_GetMousePos( & m_mouse.x, & m_mouse.y );
    }
    if ( hge->Input_GetMouseWheel() < 0 )
    {
        m_zoom -= 1;
        if ( m_zoom < 1 )
        {
            m_zoom = 1;
        }
        hge->Input_GetMousePos( & m_mouse.x, & m_mouse.y );
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
    if ( hge->Input_KeyDown( HGEK_F1 ) )
    {
        m_show_map = ! m_show_map;
    }
    if ( hge->Input_KeyDown( HGEK_1 ) )
    {
        m_mode = MODE_VIEW;
    }
    if ( hge->Input_KeyDown( HGEK_2 ) )
    {
        m_mode = MODE_TREE;
    }
    if ( hge->Input_KeyDown( HGEK_3 ) )
    {
        m_mode = MODE_BUILDING;
    }
    if ( hge->Input_KeyDown( HGEK_4 ) )
    {
        m_mode = MODE_PARKED;
    }
    if ( hge->Input_KeyDown( HGEK_5 ) )
    {
        m_mode = MODE_CAR;
    }
    if ( hge->Input_KeyDown( HGEK_6 ) )
    {
        m_mode = MODE_GUY;
    }
    float rate( dt );
    if ( hge->Input_GetKeyState( HGEK_SHIFT ) )
    {
        rate *= 10.0f;
    }
    if ( hge->Input_GetKeyState( HGEK_CTRL ) )
    {
        rate *= 0.1f;
    }
    switch ( m_mode )
    {
        case MODE_VIEW:
        {
            break;
        }
        case MODE_TREE:
        {
            if ( hge->Input_GetKeyState( HGEK_DOWN ) ||
                 hge->Input_GetKeyState( HGEK_LEFT ) )
            {
                m_radius -= 4.0f * rate;
                if ( m_radius < 1.0f )
                {
                    m_radius = 1.0f;
                }
            }
            if ( hge->Input_GetKeyState( HGEK_UP ) ||
                 hge->Input_GetKeyState( HGEK_RIGHT ) )
            {
                m_radius += 4.0f * rate;
            }
            break;
        }
        case MODE_BUILDING:
        case MODE_PARKED:
        {
            if ( hge->Input_GetKeyState( HGEK_UP ) )
            {
                m_height += 4.0f * rate;
            }
            if ( hge->Input_GetKeyState( HGEK_DOWN ) )
            {
                m_height -= 4.0f * rate;
                if ( m_height < 1.0f )
                {
                    m_height = 0.0f;
                }
            }
            if ( hge->Input_GetKeyState( HGEK_RIGHT ) )
            {
                m_width += 4.0f * rate;
            }
            if ( hge->Input_GetKeyState( HGEK_LEFT ) )
            {
                m_width -= 4.0f * rate;
                if ( m_width < 1.0f )
                {
                    m_width = 0.0f;
                }
            }
            if ( hge->Input_GetKeyState( HGEK_COMMA ) )
            {
                m_angle -= 0.1f * rate;
                if ( m_angle <= 0.0f )
                {
                    m_angle = M_PI * 2.0f;
                }
            }
            if ( hge->Input_GetKeyState( HGEK_PERIOD ) )
            {
                m_angle += 0.1f * rate;
                if ( m_angle >= M_PI * 2.0f )
                {
                    m_angle = 0.0f;
                }
            }
            break;
        }
        case MODE_CAR:
        case MODE_GUY:
        {
            if ( hge->Input_KeyDown( HGEK_UP ) ||
                 hge->Input_KeyDown( HGEK_RIGHT ) )
            {
                m_kind = 1;
            }
            else if ( hge->Input_KeyDown( HGEK_DOWN ) ||
                 hge->Input_KeyDown( HGEK_LEFT ) )
            {
                m_kind = 0;
            }
            if ( hge->Input_GetKeyState( HGEK_COMMA ) )
            {
                m_angle -= 0.1f * rate;
                if ( m_angle <= 0.0f )
                {
                    m_angle = M_PI * 2.0f;
                }
            }
            if ( hge->Input_GetKeyState( HGEK_PERIOD ) )
            {
                m_angle += 0.1f * rate;
                if ( m_angle >= M_PI * 2.0f )
                {
                    m_angle = 0.0f;
                }
            }
            break;
        }
    }
    if ( hge->Input_KeyDown( HGEK_LBUTTON ) )
    {
        hge->Input_GetMousePos( & m_mouse.x, & m_mouse.y );
    }
    if ( hge->Input_GetKeyState( HGEK_LBUTTON ) )
    {
        b2Vec2 point( 0.0f, 0.0f );
        b2Vec2 delta( 0.0f, 0.0f );
        hge->Input_GetMousePos( & point.x, & point.y );
        delta.x = point.x - m_mouse.x;
        delta.y = point.y - m_mouse.y;
        m_mouse = point;
        vp->offset().x += delta.x / vp->hscale();
        vp->offset().y += delta.y / vp->vscale();
    }
    if ( hge->Input_KeyDown( HGEK_RBUTTON ) )
    {
        b2Vec2 point( 0.0f, 0.0f );
        hge->Input_GetMousePos( & point.x, & point.y );
        vp->screenToWorld( point );
        b2Shape * shapes[1];
        b2AABB aabb;
        aabb.lowerBound = point;
        aabb.upperBound = point;
        int num( Engine::b2d()->Query( aabb, shapes, 1 ) );
        if ( num == 1 )
        {
            m_picked = static_cast< Entity * >(
                shapes[0]->GetBody()->GetUserData() );
        }
    }
    if ( hge->Input_KeyDown( HGEK_DELETE ) && m_picked != 0 )
    {
        switch ( m_picked->getType() )
        {
            case TYPE_TREE:
            {
                std::vector< Tree * >::iterator i(
                    std::find( m_trees.begin(), m_trees.end(), m_picked ) );
                m_trees.erase( i );
                break;
            }
            case TYPE_BUILDING:
            {
                std::vector< Building * >::iterator i(
                    std::find( m_buildings.begin(), m_buildings.end(),
                               m_picked ) );
                m_buildings.erase( i );
                break;
            }
            case TYPE_PARKED:
            {
                std::vector< Parked * >::iterator i(
                    std::find( m_parked.begin(), m_parked.end(), m_picked ) );
                m_parked.erase( i );
                break;
            }
            case TYPE_CAR:
            {
                std::vector< Car * >::iterator i(
                    std::find( m_cars.begin(), m_cars.end(), m_picked ) );
                m_cars.erase( i );
                break;
            }
            case TYPE_GUY:
            {
                std::vector< Guy * >::iterator i(
                    std::find( m_guys.begin(), m_guys.end(), m_picked ) );
                m_guys.erase( i );
                break;
            }
        }
        m_picked->deleteFromDatabase();
        delete m_picked;
        m_picked = 0;
    }
    if ( m_mode != MODE_VIEW && hge->Input_KeyDown( HGEK_SPACE ) )
    {
        b2Vec2 point( 0.0f, 0.0f );
        hge->Input_GetMousePos( & point.x, & point.y );
        vp->screenToWorld( point );
        Entity * entity( 0 );
        switch ( m_mode )
        {
            case MODE_TREE:
            {
                m_trees.push_back( new Tree( m_radius ) );
                entity = m_trees.back();
                break;
            }
            case MODE_BUILDING:
            {
                m_buildings.push_back( new Building( m_width, m_height ) );
                entity = m_buildings.back();
                break;
            }
            case MODE_PARKED:
            {
                m_parked.push_back( new Parked( m_width, m_height ) );
                entity = m_parked.back();
                break;
            }
            case MODE_CAR:
            {
                m_cars.push_back( new Car( m_kind ) );
                entity = m_cars.back();
                break;
            }
            case MODE_GUY:
            {
                m_guys.push_back( new Guy( m_kind ) );
                entity = m_guys.back();
                break;
            }
        }
        entity->init();
        entity->getBody()->SetXForm( point, m_angle );
        entity->persistToDatabase();
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
Editor::render()
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

    if ( m_show_map )
    {
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
    }

    b2Vec2 point( 0.0f, 0.0f );
    b2Color color( 1.0f, 0.6f, 0.6f );
    hge->Input_GetMousePos( & point.x, & point.y );
    vp->screenToWorld( point );

    DebugDraw * dd( Engine::dd() );

    switch ( m_mode )
    {
        case MODE_TREE:
        {
            dd->DrawSolidCircle( point, m_radius, point, color );
            break;
        }
        case MODE_BUILDING:
        case MODE_PARKED:
        {
            b2Vec2 vertices[4];
            vertices[0].x = - 0.5f * m_width;
            vertices[0].y = - 0.5f * m_height;
            vertices[1].x = 0.5f * m_width;
            vertices[1].y = - 0.5f * m_height;
            vertices[2].x = 0.5f * m_width;
            vertices[2].y = 0.5f * m_height;
            vertices[3].x = - 0.5f * m_width;
            vertices[3].y = 0.5f * m_height;
            b2Mat22 rotation( m_angle );
            for ( int i = 0; i < 4; ++i )
            {
                vertices[i] = b2Mul( rotation, vertices[i] ) + point;
            }
            dd->DrawSolidPolygon( vertices, 4, color );
            break;
        }
        case MODE_CAR:
        {
            hgeSprite * sprite( 0 );
            if ( m_kind == 0 )
            {
                sprite = rm->GetSprite( "car" );
            }
            else
            {
                sprite = rm->GetSprite( "ute" );
            }
            sprite->RenderEx( point.x, point.y, m_angle, 0.7f );
            break;
        }
        case MODE_GUY:
        {
            hgeSprite * sprite( 0 );
            if ( m_kind == 0 )
            {
                sprite = rm->GetSprite( "guy1" );
            }
            else
            {
                sprite = rm->GetSprite( "swat1" );
            }
            sprite->RenderEx( point.x, point.y, m_angle, 0.4f );
            break;
        }
    }

    if ( m_picked != 0 )
    {
        b2Body * body( m_picked->getBody() );
        b2Shape * shape( body->GetShapeList() );
        b2AABB aabb;
        shape->ComputeAABB( & aabb, body->GetXForm() );
        m_gui->RenderStretch( aabb.lowerBound.x, aabb.lowerBound.y,
                              aabb.upperBound.x, aabb.upperBound.y );
        hgeFont * font( rm->GetFont( "dialogue" ) );
        font->printf( body->GetPosition().x, body->GetPosition().y,
                      HGETEXT_CENTER, "[ %04d ]", m_picked->getID() );

    }

    hge->Gfx_SetTransform();
    _renderGui();
}

//------------------------------------------------------------------------------
// private
//------------------------------------------------------------------------------
void
Editor::_renderGui()
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );
    float width =
        static_cast< float >( hge->System_GetState( HGE_SCREENWIDTH ) );
    float height =
        static_cast< float >( hge->System_GetState( HGE_SCREENHEIGHT ) );
    hgeFont * font( rm->GetFont( "dialogue" ) );
    ViewPort * vp( Engine::vp() );
    b2Vec2 mouse( 0.0f, 0.0f );
    hge->Input_GetMousePos( & mouse.x, & mouse.y );
    b2Vec2 point( mouse );
    vp->screenToWorld( point );
    font->printf( 60.0f, 15.0f, HGETEXT_CENTER,
                  "(%4.2f, %4.2f)", point.x,  point.y );

    switch ( m_mode )
    {
        case MODE_VIEW:
        {
            font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "VIEW" );
            break;
        }
        case MODE_TREE:
        {
            font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "TREE" );
            font->printf( 60.0f, 45.0f, HGETEXT_CENTER, "[%4.2f]", m_radius );
            break;
        }
        case MODE_BUILDING:
        case MODE_PARKED:
        {
            if ( m_mode == MODE_BUILDING )
            {
                font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "HAUS" );
            }
            else
            {
                font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "PRKD" );
            }
            font->printf( 60.0f, 45.0f, HGETEXT_CENTER,
                "[%4.2f, %4.2f, %1.3f]", m_width, m_height, m_angle );
            break;
        }
        case MODE_CAR:
        {
            font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "VHCL" );
            if ( m_kind == 0 )
            {
                font->printf( 60.0f, 45.0f, HGETEXT_CENTER, "Car" );
            }
            else
            {
                font->printf( 60.0f, 45.0f, HGETEXT_CENTER, "Ute" );
            }
            break;
        }
        case MODE_GUY:
        {
            font->printf( 60.0f, 30.0f, HGETEXT_CENTER, "PRSN" );
            if ( m_kind == 0 )
            {
                font->printf( 60.0f, 45.0f, HGETEXT_CENTER, "Dude" );
            }
            else
            {
                font->printf( 60.0f, 45.0f, HGETEXT_CENTER, "Swat" );
            }
            break;
        }
    }

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

//==============================================================================
