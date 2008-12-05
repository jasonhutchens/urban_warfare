//==============================================================================

#ifndef ArseGame
#define ArseGame

#include <vector>

#include <hge.h>
#include <Box2D.h>

#include <context.hpp>
#include <actions.hpp>

class hgeSprite;

class Car;
class Tree;
class Guy;
class Building;
class Parked;
class Entity;

//------------------------------------------------------------------------------
// A click occurs if we hold-release within a time delta with little movement
// Double-click is two clicks in succession
class Mouse
{
  public:
    Mouse();
    virtual ~Mouse();

  private:
    Mouse( const Mouse & );
    Mouse & operator=( const Mouse & );

    enum MouseState
    {
        MOUSE_START,
        MOUSE_FIRST_DOWN,
        MOUSE_FIRST_UP,
        MOUSE_SECOND_DOWN,
        MOUSE_SECOND_UP,
        MOUSE_END
    };

    enum MouseAction
    {
        ACTION_NONE,
        ACTION_DRAGGING,
        ACTION_CLICKED,
        ACTION_DOUBLED
    };

  public:
    class MouseButton
    {
      public:
        MouseButton();
        virtual ~ MouseButton();

      private:
        MouseButton( const MouseButton & );
        MouseButton & operator=( const MouseButton & );

      public:
        void clear();
        void update( float dt, bool state );
        bool dragging() const;
        bool clicked() const;
        bool doubleClicked() const;
        const b2Vec2 & getDelta() const;

      private:
        MouseState m_state;
        MouseAction m_action;
        float m_timer;
        b2Vec2 m_last;
        b2Vec2 m_delta;
        b2Vec2 m_total;
    };

  public:
    void clear();
    void update( float dt );

    const MouseButton & getLeft();
    const MouseButton & getRight();

  private:
    MouseButton m_left;
    MouseButton m_right;
};

//------------------------------------------------------------------------------
class Game : public Context
{
  public:
    Game();
    virtual ~Game();

  private:
    Game( const Game & );
    Game & operator=( const Game & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    void _updateCars( float dt );
    void _updateGuys( float dt );
    void _updateBuildings( float dt );
    void _renderBodies();
    void _renderGuis();
    void _renderTarget( DWORD color, const b2AABB & aabb );
    void _renderGui();
    void _setViewport( Entity * entity );

  private:
    hgeSprite * m_gui;
    int m_zoom;
    std::vector< Car * > m_cars;
    std::vector< Tree * > m_trees;
    std::vector< Guy * > m_guys;
    std::vector< Building * > m_buildings;
    std::vector< Parked * > m_parked;
    Entity * m_picked;
    std::vector< Guy * > m_team;
    std::vector< Guy * > m_squad;
    ActionType m_actionType;
    bool m_lock_camera;
    Entity * m_locked;
    Mouse m_mouse;
};

#endif

//==============================================================================
