//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgesprite.h>

#include <instructions.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Instructions::Instructions()
    :
    Context()
{
}

//------------------------------------------------------------------------------
Instructions::~Instructions()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Instructions::init()
{
}

//------------------------------------------------------------------------------
void
Instructions::fini()
{
}

//------------------------------------------------------------------------------
bool
Instructions::update( float dt )
{
    HGE * hge( Engine::hge() );

    if ( hge->Input_GetKey() != 0 && ! Engine::instance()->isPaused() &&
         ! Engine::instance()->handledKey() )
    {
        Engine::instance()->switchContext( STATE_MENU );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Instructions::render()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( rm->GetSprite( "briefing_bg" ) );
    sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    sprite = rm->GetSprite( "mask" );
    sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    hgeFont * font( rm->GetFont( "menu" ) );
    font->printf( 400.0f, 80.0f, HGETEXT_CENTER,
                  "B R I E F I N G" );
    font->printf( 400.0f, 170.0f, HGETEXT_CENTER,
                  "you command an elite tactical assault group" );
    font->printf( 400.0f, 210.0f, HGETEXT_CENTER,
                  "issue commands from the safety of your home" );
    font->printf( 400.0f, 250.0f, HGETEXT_CENTER,
                  "watch everything using spy plane technology" );
    font->printf( 400.0f, 290.0f, HGETEXT_CENTER,
                  "your men are your eyes and ears on the ground" );
    font->printf( 400.0f, 330.0f, HGETEXT_CENTER,
                  "satisfy the mission objectives" );
    font->printf( 400.0f, 370.0f, HGETEXT_CENTER,
                  "minimise civilian casualties" );
    font->printf( 400.0f, 410.0f, HGETEXT_CENTER,
                  "and finish in time to collect your kids from school" );
    font->printf( 400.0f, 470.0f, HGETEXT_CENTER,
                  "W H O   D A R E S   W I N S" );
}
