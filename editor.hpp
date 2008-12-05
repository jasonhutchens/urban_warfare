//==============================================================================

#ifndef ArseEditor
#define ArseEditor

#include <vector>

#include <sqlite3.h>
#include <hge.h>
#include <Box2D.h>

#include <context.hpp>

class hgeSprite;

class Entity;
class Tree;
class Building;
class Parked;
class Car;
class Guy;

enum EditMode
{
    MODE_VIEW = 0,
    MODE_TREE = 1,
    MODE_BUILDING = 2,
    MODE_PARKED = 3,
    MODE_CAR = 4,
    MODE_GUY = 5
};

//------------------------------------------------------------------------------
class Editor : public Context
{
  public:
    Editor();
    virtual ~Editor();

  private:
    Editor( const Editor & );
    Editor & operator=( const Editor & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    void _renderGui();
    void _pickEntity( Entity * entity );

  private:
    hgeSprite * m_gui;
    int m_zoom;
    b2Vec2 m_mouse;
    float m_radius;
    float m_width;
    float m_height;
    float m_angle;
    std::vector< Tree * > m_trees;
    bool m_show_map;
    EditMode m_mode;
    std::vector< Building * > m_buildings;
    std::vector< Parked * > m_parked;
    std::vector< Car * > m_cars;
    std::vector< Guy * > m_guys;
    int m_kind;
    Entity * m_picked;
};

#endif

//==============================================================================
