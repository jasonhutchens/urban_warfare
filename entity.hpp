//==============================================================================

#ifndef ArseThing
#define ArseThing

#include <vector>

#include <sqlite3.h>

#include <actions.hpp>

//------------------------------------------------------------------------------

struct b2ContactPoint;
class hgeSprite;
class Building;
class Query;

enum EntityType
{
    TYPE_BASE = 0,
    TYPE_CAR = 1,
    TYPE_TREE = 2,
    TYPE_BUILDING = 3,
    TYPE_GUY = 4,
    TYPE_PARKED = 5
};

enum EntityAllegiance
{
    ALLEGIANCE_UNKNOWN = 0,
    ALLEGIANCE_ASSET = 1,
    ALLEGIANCE_FRIENDLY = 2,
    ALLEGIANCE_HOSTILE = 3
};

//------------------------------------------------------------------------------
class Entity : public ActionTaker
{
  public:
    Entity( float scale = 1.0f );
    virtual ~Entity();

    void init();
    void update( float dt );
    void render();
    void renderGui( hgeSprite * gui, int level, Entity * picked );

    virtual void collide( Entity * entity, b2ContactPoint * point ) = 0;
    virtual b2Body * getBody() const;

    virtual void persistToDatabase() = 0;
    virtual void deleteFromDatabase() = 0;

    void setType( EntityType type );
    EntityType getType();
    void setAllegiance( EntityAllegiance allegiance );
    EntityAllegiance getAllegiance();
    void setID( sqlite_int64 id );
    sqlite_int64 getID();
    const char * getAllegianceName();
    const char * getTypeName();
    void setVisible( bool visible );
    bool getVisible();

    virtual const b2AABB & getAABB();
    DWORD getColor();

    virtual void onActionInterrupted( Action * action );
    virtual void onActionCompleted( Action * action );

    static Entity * factory( EntityType type );
    static std::vector< Entity * > databaseFactory( EntityType type );
    static int getNextGroupIndex();
    static void resetNextGroupIndex();

  protected:
    Entity( const Entity & );
    Entity & operator=( const Entity & );

  protected:
    void persistToDatabase( char * table, char * rows[], ... );
    void deleteFromDatabase( const char * table );

    virtual void doInit() = 0;
    virtual void doUpdate( float dt ) = 0;
    virtual void doRender() = 0;
    virtual void initFromQuery( Query & query ) = 0;

  protected:
    float m_scale;
    EntityType m_type;
    sqlite_int64 m_id;
    EntityAllegiance m_allegiance;
    b2AABB m_aabb;
    bool m_visible;

  private:
    static int s_nextGroupIndex;
};

//------------------------------------------------------------------------------
class Damageable
{
  public:
    Damageable( float max_strength );
    virtual ~Damageable();

    void updateDamageable( float dt );
    void renderDamageable( const b2Vec2 & position, float scale );
    void addStrength( float amount );
    void takeDamage( float amount );
    bool isDestroyed();

  protected:
    Damageable( const Damageable & );
    Damageable & operator=( const Damageable & );

  private:
    float m_max_strength;
    float m_strength;
    float m_damage;
    float m_timer;
    hgeSprite * m_sprite;
};

//------------------------------------------------------------------------------
class Container
{
  public:
    Container( int max_size );
    virtual ~Container();

    virtual void updateContainer( float dt );
    void enter( Entity * entity );
    void leave( Entity * entity );
    void evacuate();
    int getNumOccupants();

    virtual bool allowEnter( Entity * entity ) = 0;
    virtual int getContainerGroup() = 0;
    virtual const b2AABB & getContainerBounds() = 0;
    virtual void onEnter( Entity * entity ) = 0;
    virtual void onLeave( Entity * entity ) = 0;

  protected:
    Container( const Container & );
    Container & operator=( const Container & );

  protected:
    int m_max_size;
    std::vector< Entity * > m_contents;
};

//------------------------------------------------------------------------------
class Car : public Entity, public Damageable, public Container
{
  public:
    Car( int kind = 0, float scale = 1.0f );
    virtual ~Car();

    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual b2Body * getBody() const;

    virtual void persistToDatabase();
    virtual void deleteFromDatabase();
    virtual bool allowEnter( Entity * entity );
    virtual int getContainerGroup();
    virtual const b2AABB & getContainerBounds();
    virtual void onEnter( Entity * entity );
    virtual void onLeave( Entity * entity );

  protected:
    Car( const Car & );
    Car & operator=( const Car & );

  protected:
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );

  private:
    b2Body * m_car;
    int m_kind;
};

//------------------------------------------------------------------------------
class Guy : public Entity, public Damageable
{
  public:
    Guy( int kind = 0, float scale = 1.0f );
    virtual ~Guy();

    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual b2Body * getBody() const;

    virtual void persistToDatabase();
    virtual void deleteFromDatabase();

    virtual void onActionInterrupted( Action * action );
    virtual void onActionCompleted( Action * action );

    const char * getName();
    void setLast( Entity * last );

  protected:
    Guy( const Guy & );
    Guy & operator=( const Guy & );

  protected:
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );

  private:
    void _moveAtRandom();

  private:
    b2Body * m_guy;
    int m_frame;
    float m_counter;
    int m_kind;
    char m_name[32];
    Entity * m_last;
};

//------------------------------------------------------------------------------
class Tree : public Entity
{
  public:
    Tree( float radius = 0.0f, float scale = 1.0f );
    virtual ~Tree();

    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual b2Body * getBody() const;

    virtual void persistToDatabase();
    virtual void deleteFromDatabase();

  protected:
    Tree( const Tree & );
    Tree & operator=( const Tree & );

  protected:
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );

  private:
    b2Body * m_tree;
    float m_radius;
};

//------------------------------------------------------------------------------
class MetaBuilding : public Damageable, public Container
{
  public:
    MetaBuilding();
    virtual ~MetaBuilding();

    const b2AABB & getAABB();
    void addAABB( const b2AABB & aabb );
    void setOwner( Building * building );
    Building * getOwner();
    void doUpdate( float dt );

    virtual bool allowEnter( Entity * entity );
    virtual int getContainerGroup();
    virtual const b2AABB & getContainerBounds();
    virtual void onEnter( Entity * entity );
    virtual void onLeave( Entity * entity );

  protected:
    MetaBuilding( const MetaBuilding & );
    MetaBuilding & operator=( const MetaBuilding & );

  private:
    b2AABB m_aabb;
    Building * m_owner;
};

//------------------------------------------------------------------------------
class Building : public Entity
{
  public:
    Building( float width = 0.0f, float height = 0.0f, float scale = 1.0f );
    virtual ~Building();

    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual b2Body * getBody() const;

    virtual void persistToDatabase();
    virtual void deleteFromDatabase();

    void amalgamate();
    MetaBuilding * getMeta();
    void setMeta( MetaBuilding * meta );
    virtual const b2AABB & getAABB();

  protected:
    Building( const Building & );
    Building & operator=( const Building & );

  protected:
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );

  private:
    b2Body * m_building;
    float m_width;
    float m_height;
    MetaBuilding * m_meta;
};

//------------------------------------------------------------------------------
class Parked : public Entity
{
  public:
    Parked( float width = 0.0f, float height = 0.0f, float scale = 1.0f );
    virtual ~Parked();

    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual b2Body * getBody() const;

    virtual void persistToDatabase();
    virtual void deleteFromDatabase();

  protected:
    Parked( const Parked & );
    Parked & operator=( const Parked & );

  protected:
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );

  private:
    b2Body * m_parked;
    float m_width;
    float m_height;
};

#endif

//==============================================================================
