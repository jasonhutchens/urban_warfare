//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgesprite.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>

#include <score.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Score::Score()
    :
    Context(),
    m_dark( 0 ),
    m_calculate( false ),
    m_lives( 0 ),
    m_urchins( 0 ),
    m_coins( 0 ),
    m_time( 0 ),
    m_timer( 0.0f ),
    m_buffer( 0 ),
    m_high_score(),
    m_channel( 0 )
{
}

//------------------------------------------------------------------------------
Score::~Score()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Score::init()
{
    hgeResourceManager * rm( Engine::rm() );
    HMUSIC music = rm->GetMusic( "theme" );
    HGE * hge( Engine::hge() );
    hge->Music_SetAmplification( music, 20 );
    m_dark = new hgeSprite( 0, 0, 0, 1, 1 );
    m_calculate = false;
    m_channel = 0;
    _updateScore();
}

//------------------------------------------------------------------------------
void
Score::fini()
{
    HMUSIC music = Engine::rm()->GetMusic( "theme" );
    HGE * hge( Engine::hge() );
    hge->Music_SetAmplification( music, 50 );
    hge->Channel_Stop( m_channel );
    m_channel = 0;
    delete m_dark;
    m_dark = 0;
}

//------------------------------------------------------------------------------
bool
Score::update( float dt )
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );
    hgeParticleManager * pm( Engine::pm() );
    
    if ( hge->Input_GetKey() != 0 &&
         ! Engine::instance()->isPaused() && ! m_calculate &&
         ! Engine::instance()->handledKey() )
    {
        Engine::instance()->switchContext( STATE_MENU );
        return false;
    }

    if ( Engine::instance()->isPaused() )
    {
        return false;
    }

    if ( m_calculate )
    {
        m_timer += dt;
        if ( static_cast< int >( m_timer * 1000.0f ) % 2 == 0 )
        {
            if ( m_buffer > 0 )
            {
                m_buffer -= 1;
                m_coins += 1;
                float x( hge->Random_Float( 550.0f, 560.0f ) );
                float y( hge->Random_Float( 268.0f, 278.0f ) );
                hgeParticleSystem * particle =
                    pm->SpawnPS(& rm->GetParticleSystem("collect")->info, x, y);
                if ( particle != 0 )
                {
                    particle->SetScale( 1.0f );
                }
                hge->Effect_Play( rm->GetEffect( "bounce" ) );
            }
            else if ( m_lives > 0 )
            {
                m_lives -= 1;
                m_buffer += 100;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
            else if ( m_urchins > 0 )
            {
                m_urchins -= 1;
                m_buffer += 10;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
            else if ( m_time > 0 )
            {
                m_time -= 7;
                if ( m_time < 0 )
                {
                    m_time = 0;
                }
                m_buffer += 1;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
        }
        if ( m_buffer == 0 && m_lives == 0 && m_urchins == 0 && m_time == 0 )
        {
            int character( hge->Input_GetChar() );
            if ( character != 0 )
            {
                if ( ( character == ' ' ||
                       character == '.' ||
                       character == '!' ||
                       character == '?' ||
                       ( character >= '0' && character <= '9' ) ||
                       ( character >= 'a' && character <= 'z' ) ||
                       ( character >= 'A' && character <= 'Z' ) ) && 
                     m_name.size() <= 15 )
                {
                    m_name.push_back( character );
                }
            }
            if ( hge->Input_KeyDown( HGEK_BACKSPACE ) ||
                 hge->Input_KeyDown( HGEK_DELETE ) )
            {
                if ( m_name.size() > 0 )
                {
                    m_name.erase( m_name.end() - 1 );
                }
            }
            if ( hge->Input_KeyDown( HGEK_ENTER ) )
            {
                if ( m_name.size() == 0 )
                {
                    m_name = "Anonymous";
                }
                Database db( "world.db3" );
                Query q( db );
                char query[1024];
                sprintf_s( query, 1024, "INSERT INTO score(name, coins) "
                           "VALUES('%s',%d)", m_name.c_str(), m_coins );
                q.execute( query );
                _updateScore();
                m_calculate = false;
            }
        }
    }
    else if ( m_channel == 0 )
    {
        m_channel = Engine::hge()->Effect_PlayEx( rm->GetEffect( "bugle" ),
                                                  100, 0, 1 );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Score::render()
{
    hgeResourceManager * rm( Engine::rm() );

    if ( m_calculate )
    {
        hgeSprite * sprite( rm->GetSprite( "over_bg" ) );
        sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
        sprite = rm->GetSprite( "mask" );
        sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    }
    else
    {
        hgeSprite * sprite( rm->GetSprite( "score_bg" ) );
        sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
        sprite = rm->GetSprite( "mask" );
        sprite->RenderEx( 400.0f, 300.0f, 0.0f, 2.0f );
    }

    hgeFont * font( 0 );

    if ( m_calculate )
    {
        font = rm->GetFont( "menu" );
        font->printf( 400.0f, 80.0f, HGETEXT_CENTER, "G A M E   O V E R" );
        font->printf( 200.0f, 200.0f, HGETEXT_LEFT, "x %d", m_lives );
        font->printf( 200.0f, 260.0f, HGETEXT_LEFT, "x %02d/99", m_urchins );
        font->printf( 200.0f, 320.0f, HGETEXT_LEFT, "%03d", m_time );
        font->printf( 580.0f, 260.0f, HGETEXT_LEFT, "x %04d", m_coins );
        rm->GetSprite( "ship" )->Render( 175.0f, 213.0f );
        rm->GetSprite( "coin")->SetColor( 0xFFFFFFFF );
        rm->GetSprite( "coin" )->Render( 555.0f, 273.0f );
        rm->GetSprite( "urchin_green" )->Render( 175.0f, 273.0f );
        if ( m_buffer == 0 && m_lives == 0 && m_urchins == 0 && m_time == 0 )
        {
            font->printf( 400.0f, 400.0f, HGETEXT_CENTER,
                          "%s", m_name.c_str() );
            font->printf( 400.0f, 500.0f, HGETEXT_CENTER,
                          "(well done, you)" );
            if ( static_cast<int>( m_timer * 2.0f ) % 2 != 0 )
            {
                float width = font->GetStringWidth( m_name.c_str() );
                m_dark->SetColor( 0xFFFFFFFF );
                m_dark->RenderStretch( 400.0f + width * 0.5f, 425.0f,
                                       400.0f + width * 0.5f + 16.0f, 427.0f );
            }
        }
    }
    else
    {
        font = rm->GetFont( "menu" );
        font->printf( 400.0f, 60.0f, HGETEXT_CENTER,
                      "L E S T   W E   F O R G E T" );
        font = rm->GetFont( "menu" );
        int i = 0;
        std::vector< std::pair< std::string, int > >::iterator j;
        for ( j = m_high_score.begin(); j != m_high_score.end(); ++j )
        {
            i += 1;
            font->printf( 200.0f, 120.0f + i * 30.0f, HGETEXT_LEFT,
                          "(%d)", i );
            font->printf( 400.0f, 120.0f + i * 30.0f, HGETEXT_CENTER,
                          "%s", j->first.c_str() );
            font->printf( 600.0f, 120.0f + i * 30.0f, HGETEXT_RIGHT,
                          "%04d", j->second );
        }
        font->printf( 400.0f, 490.0f, HGETEXT_CENTER,
                      "they gave their lives so we may live in peace" );

    }
}

//------------------------------------------------------------------------------
void
Score::calculateScore( int lives, int urchins, int coins, int time )
{
    m_timer = 0.0f;
    m_buffer = 0;
    m_calculate = true;
    m_lives = lives;
    m_urchins = urchins;
    m_coins = coins;
    m_time = time;
    if ( lives == 0 || urchins == 0 )
    {
        m_time = 0;
    }
    m_name.clear();
    if ( m_lives == 0 && m_time == 0 && m_coins == 0 && m_urchins == 0 )
    {
        m_calculate = false;
    }
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Score::_updateScore()
{
    m_high_score.clear();

    Database db( "world.db3" );
    Query q( db );

    q.get_result("SELECT coins, name FROM score ORDER BY coins DESC LIMIT 10");

    while ( q.fetch_row() )
    {
        std::pair< std::string, int > pair( q.getstr(), q.getval() );
        m_high_score.push_back( pair );
    }

    q.free_result();
}
