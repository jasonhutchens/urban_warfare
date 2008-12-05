//==============================================================================

#include <hgeresource.h>
#include <hgedistort.h>

#include <splash.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Splash::Splash()
    :
    Context(),
    m_channel( 0 ),
    m_timer( 0.0f ),
    m_delta_time( 0.0f )
{
}

//------------------------------------------------------------------------------
Splash::~Splash()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Splash::init()
{
    hgeResourceManager * rm( Engine::rm() );
    m_timer = 0.0f;
    HMUSIC music = rm->GetMusic( "theme" );
    m_channel = Engine::hge()->Music_Play( music, true, 100, 0, 0 );
    Engine::instance()->setColour( 0xFFFFFFFF );
    Engine::instance()->hideMouse();
}

//------------------------------------------------------------------------------
void
Splash::fini()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeFont * font( rm->GetFont( "menu" ) );
    font->SetColor( 0xFFFFFFFF );
}

//------------------------------------------------------------------------------
bool
Splash::update( float dt )
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );

    m_delta_time = dt;

    bool paused( Engine::instance()->isPaused() );
    if ( paused && hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Pause( m_channel );
    }
    else if ( ! paused && ! hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Resume( m_channel );
    }

    m_timer = hge->Channel_GetPos( m_channel );

    if ( ( m_timer > 28.0 || ( hge->Input_KeyDown( HGEK_BACKSPACE ) &&
                               ! Engine::instance()->handledKey() ) ) && 
         ! Engine::instance()->isPaused() )
    {
        Engine::instance()->switchContext( STATE_MENU );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Splash::render()
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );

    int width( hge->System_GetState( HGE_SCREENWIDTH ) );
    int height( hge->System_GetState( HGE_SCREENHEIGHT ) );

    if ( m_timer < 3.8f )
    {
        Engine::instance()->setColour( 0xFFFFFFFF );
        _fade( 0.0f, 2.0f, 2.0f, 3.8f, "publisher" );
    }
    else if ( m_timer < 8.0f )
    {
        Engine::instance()->setColour( 0xFFFFFFFF );
        _fade( 3.8f, 4.8f, 6.0f, 8.0f, "developer" );
    }
    else
    {
        if ( m_timer < 12.0f )
        {
            unsigned char level( static_cast<unsigned char>( 255.0f *
                                 0.25f * ( 12.0f - m_timer ) ) );
            Engine::instance()->setColour( ARGB( 0xFF, level, level, level ) );
        }
        else
        {
            Engine::instance()->setColour( 0x00000000 );
        }
        if ( m_timer < 14.0f )
        {
            hgeFont * font( rm->GetFont( "menu" ) );
            font->printf( 400.0f, 270.0f, HGETEXT_CENTER,
                "This game is not in any way endorsed by" );
            font->printf( 400.0f, 300.0f, HGETEXT_CENTER,
                "the Australian Tactical Assault Group." );
        }
        if ( m_timer > 13.0f )
        {
            _fade( 13.0f, 14.0f, 26.0f, 28.0f, "game", 1.0f );
        }
        if ( m_timer > 15.5 && m_timer < 20.0f )
        {
            hgeFont * font( rm->GetFont( "menu" ) );
            if ( m_timer < 18.0f )
            {
                font->SetColor( 0xFFFFFFFF );
            }
            else
            {
                unsigned char level( static_cast<unsigned char>( 255.0f *
                                     0.5f * ( 20.0f - m_timer ) ) );
                font->SetColor( ARGB( 0xFF, level, level, level ) );
            }
            font->printf( 150.0f, 275.0f + 10.0f * ( 20.0f - m_timer ),
                          HGETEXT_CENTER, "Lloyd Kranzky" );
        }
        if ( m_timer > 20.5 && m_timer < 25.0f )
        {
            hgeFont * font( rm->GetFont( "menu" ) );
            if ( m_timer < 23.0f )
            {
                font->SetColor( 0xFFFFFFFF );
            }
            else
            {
                unsigned char level( static_cast<unsigned char>( 255.0f *
                                     0.5f * ( 25.0f - m_timer ) ) );
                font->SetColor( ARGB( 0xFF, level, level, level ) );
            }
            font->printf( 650.0f, 275.0f + 10.0f * ( 25.0f - m_timer ),
                          HGETEXT_CENTER, "Pazu Kranzky" );
        }
    }
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool
Splash::_onTime( float time )
{
    return time >= m_timer && time <= m_timer + m_delta_time;
}

//------------------------------------------------------------------------------
void
Splash::_fade( float start_in, float start_out, float end_in, float end_out,
               const char * name, float scale )
{
    hgeResourceManager * rm( Engine::rm() );
    hgeColorRGB color( 0xFFFFFFFF );

    int width( Engine::hge()->System_GetState( HGE_SCREENWIDTH ) );
    int height( Engine::hge()->System_GetState( HGE_SCREENHEIGHT ) );

    if ( m_timer > start_in && m_timer < start_out )
    {
        color.a = ( m_timer - start_in ) / ( start_out - start_in );
    }
    else if ( m_timer > end_in && m_timer < end_out )
    {
        color.a = 1.0f - ( m_timer - end_in ) / ( end_out - end_in );
    }

    hgeSprite * sprite( rm->GetSprite( name ) );
    sprite->SetColor( color.GetHWColor() );
    sprite->RenderEx( 0.5f * static_cast<float>( width ),
                      0.5f * static_cast<float>( height ), 0.0f, scale );
}

//==============================================================================
