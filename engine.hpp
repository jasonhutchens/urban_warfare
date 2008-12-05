//==============================================================================

#ifndef ArseEngine
#define ArseEngine

#include <vector>

#include <hge.h>
#include <Box2D.h>

class hgeResourceManager;
class hgeParticleManager;
class hgeSprite;

class DebugDraw;
class Context;
class ViewPort;

//------------------------------------------------------------------------------
enum EngineState
{
    STATE_NONE = -1, 
    STATE_SPLASH = 0,
    STATE_MENU = 1,
    STATE_GAME = 2,
    STATE_SCORE = 3,
    STATE_EDITOR = 4,
    STATE_INSTRUCTIONS = 5
};

//------------------------------------------------------------------------------
class Engine : public b2BoundaryListener, public b2ContactListener
{
  public:
    static Engine * instance();
    static HGE * hge();
    static b2World * b2d();
    static ViewPort * vp();
    static hgeResourceManager * rm();
    static hgeParticleManager * pm();
    static DebugDraw * dd();

  private:
    static bool s_update();
    static bool s_render();
    
  protected:
    Engine();
    Engine( const Engine & );
    Engine & operator=( const Engine & );
    ~Engine();

  public:
    bool handledKey();
    bool isPaused();
    bool isDebug();
    float getTimeRatio();
    void error( const char * format, ... );
    void start();
    void switchContext( EngineState state );
    Context * getContext();
    void setColour( DWORD colour );
    void showMouse();
    void setMouse( const char * name );
    void hideMouse();
    virtual void Violation( b2Body * body );
    virtual void Add( b2ContactPoint * point );
    virtual void Persist( b2ContactPoint * point );
    virtual void Remove( b2ContactPoint * point );

  private:
    bool _update();
    void _pauseOverlay();
    bool _render();
    void _initGraphics();
    void _initPhysics();
    void _loadData();

  private:
    static Engine * s_instance;
    hgeResourceManager * m_rm;
    hgeParticleManager * m_pm;
    HGE * m_hge;
    b2World * m_b2d;
    ViewPort * m_vp;
    DWORD m_colour;
    DebugDraw * m_dd;
    hgeSprite * m_overlay;
    std::vector< Context * > m_contexts;
    EngineState m_state;
    bool m_handled_key;
    bool m_paused;
    bool m_running;
    bool m_mouse;
    hgeSprite * m_mouse_sprite;
    float m_time_ratio;
};

#endif

//==============================================================================
