//==============================================================================

#ifndef ArseMenu
#define ArseMenu

#include <hge.h>
#include <hgegui.h>

#include <context.hpp>

class hgeSprite;
class hgeFont;
class hgeGUI;

enum Control
{
    CTRL_NONE,
    CTRL_TITLES,
    CTRL_INSTRUCTIONS,
    CTRL_START,
    CTRL_SCORE,
    CTRL_EDITOR,
    CTRL_EXIT
};

//------------------------------------------------------------------------------
class Menu : public Context
{
  public:
    Menu();
    virtual ~Menu();

  private:
    Menu( const Menu & );
    Menu & operator=( const Menu & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    void _stopMusic();

  private:
    hgeSprite * m_cursor;
    hgeFont * m_font;
    hgeGUI * m_gui;
};

//------------------------------------------------------------------------------
class MenuItem : public hgeGUIObject
{
  public:
    MenuItem( Control control, float x, float y, const char * title,
              hgeFont * font ); 

    virtual void    Render();
    virtual void    Update( float dt );
    virtual void    Enter();
    virtual void    Leave();
    virtual bool    IsDone();
    virtual void    Focus( bool focused );
    virtual void    MouseOver( bool over );
    virtual bool    MouseLButton( bool down );
    virtual bool    KeyClick( int key, int chr );

  private:
    hgeFont * m_font;
    const char * m_title;
};

#endif

//==============================================================================
