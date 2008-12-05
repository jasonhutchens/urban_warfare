//==============================================================================

#include <stdarg.h>

#include <hgesprite.h>
#include <hgeanim.h>
#include <hgeresource.h>

#include <engine.hpp>
#include <splash.hpp>
#include <menu.hpp>
#include <game.hpp>
#include <score.hpp>
#include <editor.hpp>
#include <instructions.hpp>
#include <debug.hpp>
#include <entity.hpp>
#include <viewport.hpp>

//------------------------------------------------------------------------------

Engine * Engine::s_instance( 0 );

//------------------------------------------------------------------------------
Engine::Engine()
    :
    m_rm( 0 ),
    m_pm( 0 ),
    m_hge( 0 ),
    m_b2d( 0 ),
    m_vp( 0 ),
    m_colour( 0 ),
    m_dd( 0 ),
    m_overlay( 0 ),
    m_contexts(),
    m_state( STATE_NONE ),
    m_handled_key( false ),
    m_paused( false ),
    m_running( false ),
    m_mouse( false ),
    m_mouse_sprite( 0 ),
    m_time_ratio( 1.0f )
{
    m_vp = new ViewPort();
}

//------------------------------------------------------------------------------
Engine::~Engine()
{
    std::vector< Context * >::iterator i;
    for ( i = m_contexts.begin(); i != m_contexts.end(); ++i )
    {
        Context * context( * i );
        context->fini();
        delete context;
    }
    m_contexts.clear();

    delete m_pm;
    m_pm = 0;

    m_rm->Purge();
    delete m_rm;
    m_rm = 0;

    if ( m_hge != 0 )
    {
        m_hge->System_Shutdown();
        m_hge->Release();
        m_hge = 0;
    }

    delete m_b2d;
    delete m_dd;
    delete m_overlay;
    delete m_vp;
}

//------------------------------------------------------------------------------
bool
Engine::handledKey()
{
    return m_handled_key;
}

//------------------------------------------------------------------------------
bool
Engine::isPaused()
{
    return m_paused;
}

//------------------------------------------------------------------------------
bool
Engine::isDebug()
{
    return m_dd->GetFlags() != 0;
}

//------------------------------------------------------------------------------
float
Engine::getTimeRatio()
{
    return m_time_ratio;
}

//------------------------------------------------------------------------------
void
Engine::error( const char * format, ... )
{
    char message[1024];

    va_list ap;
    va_start( ap, format );
    vsprintf_s( message, 1024, format, ap );
    va_end( ap );

    m_hge->System_Log( "Error: %s", message );
    MessageBox( NULL, message, "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

//------------------------------------------------------------------------------
void
Engine::start()
{
    m_contexts.push_back( new Splash() );
    m_contexts.push_back( new Menu() );
    m_contexts.push_back( new Game() );
    m_contexts.push_back( new Score() );
    m_contexts.push_back( new Editor() );
    m_contexts.push_back( new Instructions() );

    m_pm = new hgeParticleManager();

    _initGraphics();
    _initPhysics();

    if ( m_hge->System_Initiate() )
    {
        _loadData();
        switchContext( STATE_SPLASH );
        m_hge->Random_Seed();
        m_hge->System_Start();
    }
    else
    {
        MessageBox( NULL, m_hge->System_GetErrorMessage(), "Error",
                    MB_OK | MB_ICONERROR | MB_APPLMODAL );
    }
}

//------------------------------------------------------------------------------
void
Engine::switchContext( EngineState state )
{
    m_running = false;
    m_colour = 0;

    if ( m_state != STATE_NONE )
    {
        m_contexts[m_state]->fini();
    }

    m_pm->KillAll();
    hgeInputEvent event;
    while ( m_hge->Input_GetEvent( & event ) );
    hideMouse();

    m_state = state;
    m_paused = false;
    m_handled_key = false;

    int flags( b2DebugDraw::e_shapeBit |
               b2DebugDraw::e_aabbBit |
               b2DebugDraw::e_obbBit );
    m_dd->ClearFlags( flags );

    if ( m_state != STATE_NONE )
    {
        m_contexts[m_state]->init();
    }

    m_running = true;
}

//------------------------------------------------------------------------------
Context *
Engine::getContext()
{
    return m_contexts[m_state];
}

//------------------------------------------------------------------------------
void
Engine::setColour( DWORD colour )
{
    m_colour = colour;
}

//------------------------------------------------------------------------------
void
Engine::showMouse()
{
    m_mouse = true;
}

//------------------------------------------------------------------------------
void
Engine::setMouse( const char * name )
{
    m_mouse_sprite = m_rm->GetSprite( name );
}

//------------------------------------------------------------------------------
void
Engine::hideMouse()
{
    m_mouse = false;
}

//------------------------------------------------------------------------------
// physics:
//------------------------------------------------------------------------------
void
Engine::Violation( b2Body * body )
{
    m_hge->System_Log( "Body left world" );
    Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
    // TODO: delete object here
}

//------------------------------------------------------------------------------
void
Engine::Add( b2ContactPoint * point )
{
    Entity * entity1 =
        static_cast< Entity * >( point->shape1->GetBody()->GetUserData() );
    Entity * entity2 =
        static_cast< Entity * >( point->shape2->GetBody()->GetUserData() );
    entity1->collide( entity2, point );
    entity2->collide( entity1, point );
}

//------------------------------------------------------------------------------
void
Engine::Persist( b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
Engine::Remove( b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
//static:
//------------------------------------------------------------------------------
Engine *
Engine::instance()
{
    if ( s_instance == 0 )
    {
        s_instance = new Engine;
    }
    return s_instance;
}

//------------------------------------------------------------------------------
HGE *
Engine::hge()
{
    return instance()->m_hge;
}

//------------------------------------------------------------------------------
b2World *
Engine::b2d()
{
    return instance()->m_b2d;
}

//------------------------------------------------------------------------------
ViewPort *
Engine::vp()
{
    return instance()->m_vp;
}

//------------------------------------------------------------------------------
hgeResourceManager *
Engine::rm()
{
    return instance()->m_rm;
}

//------------------------------------------------------------------------------
hgeParticleManager *
Engine::pm()
{
    return instance()->m_pm;
}

//------------------------------------------------------------------------------
DebugDraw *
Engine::dd()
{
    return instance()->m_dd;
}

//------------------------------------------------------------------------------
//private:
//------------------------------------------------------------------------------
bool
Engine::s_update()
{
    return s_instance->_update();
}

//------------------------------------------------------------------------------
bool
Engine::s_render()
{
    return s_instance->_render();
}

//------------------------------------------------------------------------------
bool
Engine::_update()
{
    float dt( m_hge->Timer_GetDelta() );

    if ( m_hge->Input_KeyDown( HGEK_P ) && m_state != STATE_SCORE )
    {
        m_handled_key = true;
        m_paused = ! m_paused;
    }

    if ( m_hge->Input_KeyDown( HGEK_O ) && m_state != STATE_SCORE  )
    {
        m_handled_key = true;
        int flags( b2DebugDraw::e_shapeBit |
                   b2DebugDraw::e_aabbBit |
                   b2DebugDraw::e_obbBit );
        if ( m_dd->GetFlags() != 0 )
        {
            m_dd->ClearFlags( flags );
            m_time_ratio = 1.0f;
        }
        else
        {
            m_dd->SetFlags( flags );
        }
    }

    if ( m_dd->GetFlags() != 0 )
    {
        if ( m_hge->Input_KeyDown( HGEK_EQUALS ) )
        {
            m_time_ratio /= 0.9f;
        }
        if ( m_hge->Input_KeyDown( HGEK_MINUS ) )
        {
            m_time_ratio *= 0.9f;
        }
        dt *= m_time_ratio;
    }

    if ( m_dd->GetFlags() != 0 )
    {
        m_hge->Gfx_BeginScene();
        m_hge->Gfx_Clear( 0 );
        m_contexts[m_state]->render();
        m_hge->Gfx_SetTransform( 400.0f,
                                 300.0f,
                                 m_vp->offset().x * m_vp->hscale(),
                                 m_vp->offset().y * m_vp->vscale(),
                                 m_vp->angle(),
                                 m_vp->hscale(),
                                 m_vp->vscale() );
    }      

    if ( m_paused )
    {
        dt = 0.0f;
    }

    m_b2d->Step( dt, 10 );
    bool retval( m_contexts[m_state]->update( dt ) );
    m_pm->Update( dt );

    if ( m_dd->GetFlags() != 0 )
    {
        m_hge->Gfx_SetTransform();
        _pauseOverlay();
        m_hge->Gfx_EndScene();
    }  

    return retval;
}

//------------------------------------------------------------------------------
bool
Engine::_render()
{
    if ( ! m_running )
    {
        return false;
    }

    if ( m_dd->GetFlags() == 0 )
    {
        m_hge->Gfx_BeginScene();
        m_hge->Gfx_Clear( m_colour );
        m_contexts[m_state]->render();
        m_hge->Gfx_SetTransform();
        if ( m_mouse && m_mouse_sprite != 0 )
        {
            float x( 0.0f ); 
            float y( 0.0f );
            m_hge->Input_GetMousePos( & x, & y );
            m_mouse_sprite->Render( x, y );
        }
        _pauseOverlay();
        m_hge->Gfx_EndScene();
    }

    return false;
}

//------------------------------------------------------------------------------
void
Engine::_pauseOverlay()
{
    if ( ! m_paused && m_dd->GetFlags() == 0 )
    {
        return;
    }

    hgeFont * font( m_rm->GetFont( "menu" ) );
    float width =
        static_cast< float >( m_hge->System_GetState( HGE_SCREENWIDTH ) );
    float height =
        static_cast< float >( m_hge->System_GetState( HGE_SCREENHEIGHT ) );
    if ( m_paused )
    {
        m_overlay->RenderStretch( 0.0f, 0.0f, width, height );
        font->Render( width / 2.0f, 0.0f, HGETEXT_CENTER,
                      "+++ P A U S E D +++" );
    }
    if ( m_dd->GetFlags() != 0 )
    {
        font->Render( width / 2.0f, height - font->GetHeight(), HGETEXT_CENTER,
                      "+++ D E B U G +++" );
    }
}

//------------------------------------------------------------------------------
void
Engine::_initGraphics()
{
    m_hge = hgeCreate( HGE_VERSION );
    m_hge->System_SetState( HGE_LOGFILE, "kranzky.log" );
    m_hge->System_SetState( HGE_FRAMEFUNC, s_update );
    m_hge->System_SetState( HGE_RENDERFUNC, s_render );
    m_hge->System_SetState( HGE_TITLE, "+++ U R B A N | W A R F A R E +++" );
    m_hge->System_SetState( HGE_WINDOWED, true );
    m_hge->System_SetState( HGE_SCREENWIDTH, 800 );
    m_hge->System_SetState( HGE_SCREENHEIGHT, 600 );
    m_hge->System_SetState( HGE_SCREENBPP, 32 );
    m_hge->System_SetState( HGE_USESOUND, true );
    m_hge->System_SetState( HGE_SHOWSPLASH, false );
    m_hge->System_SetState( HGE_FPS, HGEFPS_VSYNC );

    m_overlay = new hgeSprite( 0, 0, 0, 1, 1 );
    m_overlay->SetColor( 0xBB000000 );
}

//------------------------------------------------------------------------------
void
Engine::_initPhysics()
{
    b2AABB worldAABB;
    worldAABB.lowerBound.Set( -2500.0f, -2500.0f );
    worldAABB.upperBound.Set( 2500.0f, 2500.0f );
    b2Vec2 gravity( 0.0f, 0.0f );
    m_b2d = new b2World( worldAABB, gravity, true );
    m_dd = new DebugDraw( m_hge, m_vp );
    m_b2d->SetDebugDraw( m_dd );
    m_b2d->SetListener( static_cast< b2ContactListener *>( this ) );
    m_b2d->SetListener( static_cast< b2BoundaryListener *>( this ) );
    m_vp->screen().x = 800.0f;
    m_vp->screen().y = 600.0f;
    m_vp->offset().x = 0.0f;
    m_vp->offset().y = 0.0f;
    m_vp->bounds().x = 8.0f;
    m_vp->bounds().y = 6.0f;
}

//------------------------------------------------------------------------------
void
Engine::_loadData()
{
    if ( ! m_hge->Resource_AttachPack( "resources.dat" ) )
    {
        error( "Cannot load '%s'", "resources.dat" );
    }

    m_rm = new hgeResourceManager( "data.res" );
    m_rm->Precache();

    m_hge->Resource_RemovePack( "resources.dat" );
}

//==============================================================================
