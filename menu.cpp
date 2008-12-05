//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgegui.h>
#include <hgesprite.h>

#include <menu.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Menu::Menu()
    :
    Context(),
    m_cursor( 0 ),
    m_font( 0 ),
    m_gui( 0 )
{
}

//------------------------------------------------------------------------------
Menu::~Menu()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Menu::init()
{
    hgeResourceManager * rm( Engine::rm() );

    m_font = rm->GetFont( "menu" );
    m_cursor = rm->GetSprite( "cursor" );
    m_gui = new hgeGUI();
    m_gui->AddCtrl( new MenuItem( CTRL_TITLES, 400, 180, "Flashbacks", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_INSTRUCTIONS, 400, 240, "Briefing",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_START, 400, 270, "Enter the Fray",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, 400, 300, "Lest We Forget",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EDITOR, 400, 330, "Cartography",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EXIT, 400, 390, "AWOL", m_font ) );
    m_gui->SetNavMode( HGEGUI_UPDOWN | HGEGUI_CYCLED );
    m_gui->SetCursor( m_cursor );
    m_gui->SetFocus( 1 );
    m_gui->Enter();
    HMUSIC music = rm->GetMusic( "theme" );
    int order( 0 );
    int row( 0 );
    Engine::hge()->Music_GetPos( music, & order, & row );
    if ( order == 0 && row == 0 )
    {
        Engine::hge()->Music_Play( music, true, 100, 0, 0 );
    }
    Engine::instance()->setMouse( "cursor" );
    Engine::instance()->showMouse();
}

//------------------------------------------------------------------------------
void
Menu::fini()
{
    m_gui->DelCtrl( CTRL_TITLES );
    m_gui->DelCtrl( CTRL_INSTRUCTIONS );
    m_gui->DelCtrl( CTRL_START );
    m_gui->DelCtrl( CTRL_SCORE );
    m_gui->DelCtrl( CTRL_EDITOR );
    m_gui->DelCtrl( CTRL_EXIT );
    delete m_gui;
    m_gui = 0;
    m_font = 0;
    m_cursor = 0;
}

//------------------------------------------------------------------------------
bool
Menu::update( float dt )
{
    HGE * hge( Engine::hge() );

    switch ( static_cast< Control >( m_gui->Update( dt ) ) )
    {
        case CTRL_TITLES:
        {
            _stopMusic();
            Engine::instance()->switchContext( STATE_SPLASH );
            break;
        }
        case CTRL_INSTRUCTIONS:
        {
            Engine::instance()->switchContext( STATE_INSTRUCTIONS );
            break;
        }
        case CTRL_START:
        {
            _stopMusic();
            Engine::instance()->switchContext( STATE_GAME );
            break;
        }
        case CTRL_SCORE:
        {
            Engine::instance()->switchContext( STATE_SCORE );
            break;
        }
        case CTRL_EDITOR:
        {
            _stopMusic();
            Engine::instance()->switchContext( STATE_EDITOR );
            break;
        }
        case CTRL_EXIT:
        {
            _stopMusic();
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void
Menu::render()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( rm->GetSprite( "menu_bg" ) );
    sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    sprite = rm->GetSprite( "mask" );
    sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    m_gui->Render();
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Menu::_stopMusic()
{
    Engine::hge()->Channel_StopAll();
    HMUSIC music = Engine::rm()->GetMusic( "theme" );
    Engine::hge()->Music_SetPos( music, 0, 0 );
}

//==============================================================================
MenuItem::MenuItem( Control control, float x, float y, const char * title,
                    hgeFont * font )
    :
    hgeGUIObject(),
    m_title( title ),
    m_font( font )

{
    id = static_cast<int>( control );

    bStatic=false;
    bVisible=true;
    bEnabled=true;

    float width( m_font->GetStringWidth( title ) );
    rect.Set( x - width / 2, y, x + width / 2, y + m_font->GetHeight() );
}
 
//------------------------------------------------------------------------------
void
MenuItem::Render()
{
    m_font->Render( rect.x1, rect.y1, HGETEXT_LEFT, m_title );
}

//------------------------------------------------------------------------------
void
MenuItem::Update( float dt )
{
}

//------------------------------------------------------------------------------
void
MenuItem::Enter()
{
}

//------------------------------------------------------------------------------
void
MenuItem::Leave()
{
}

//------------------------------------------------------------------------------
bool
MenuItem::IsDone()
{
    return true;
}

//------------------------------------------------------------------------------
void
MenuItem::Focus( bool focused )
{
}

//------------------------------------------------------------------------------
void
MenuItem::MouseOver( bool over )
{
    if ( over )
    {
        gui->SetFocus( id );
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::MouseLButton( bool down )
{
    if ( down )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::KeyClick( int key, int chr )
{
    if ( key == HGEK_ENTER || key == HGEK_SPACE )
    {
        MouseLButton( true );
        return MouseLButton( false );
    }

    return false;
}

//==============================================================================
