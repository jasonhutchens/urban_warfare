//==============================================================================

#ifndef ArseDo
#define ArseDo

#include <vector>
#include <map>

class Entity;
class Action;
class Target;

enum ActionType
{
    TYPE_NONE = 0,
    TYPE_MOVE = 1,
    TYPE_HALT = 2,
    TYPE_INVESTIGATE = 3,
    TYPE_RESCUE = 4,
    TYPE_ATTACK = 5,
    TYPE_AIRSTRIKE = 6
};

//------------------------------------------------------------------------------
class ActionTaker
{
  public:
    ActionTaker();
    virtual ~ActionTaker();

    bool hasAction( ActionType type );
    Action * getAction( ActionType type );
    void addAction( Action * action );
    void stopAction( Action * action );

  protected:
    ActionTaker( const ActionTaker & );
    ActionTaker & operator=( const ActionTaker & );

  protected:
    void collideActions( Entity * entity, b2ContactPoint * point );
    void updateActions( float dt );
    void renderActions();

  protected:
    ActionType m_supported;
    Entity * m_entity;

  private:
    std::map< ActionType, Action * > m_actions;
};

//------------------------------------------------------------------------------
class Action
{
  public:
    Action( Target * target );
    virtual ~Action();

    virtual void init() = 0;
    virtual void collide( Entity * entity, b2ContactPoint * point ) = 0;
    virtual void update( float dt ) = 0;
    virtual void render() = 0;

    static Action * factory( ActionType type, Target * target );

    bool isComplete();

    ActionType getType();
    Target * getTarget();

  protected:
    Action( const Action & );
    Action & operator=( const Action & );

  protected:
    friend class ActionTaker;
    void setEntity( Entity * entity );
    Entity * getEntity();

  protected:
    Entity * m_entity;
    Target * m_target;
    ActionType m_type;
    bool m_complete;
};

//------------------------------------------------------------------------------
class Target
{
  public:
    Target( Entity * entity );
    Target( b2Vec2 position );
    virtual ~Target();

    const b2Vec2 & getPosition();
    Entity * getEntity();

  protected:
    Target( const Target & );
    Target & operator=( const Target & );

  private:
    Entity * m_entity;
    b2Vec2 m_position;
};

//------------------------------------------------------------------------------
class MoveAction : public Action
{
  public:
    MoveAction( Target * target );
    virtual ~MoveAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    MoveAction( const MoveAction & );
    MoveAction & operator=( const MoveAction & );

  private:
    void _completeAction();
};

//------------------------------------------------------------------------------
class HaltAction : public Action
{
  public:
    HaltAction( Target * target );
    virtual ~HaltAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    HaltAction( const HaltAction & );
    HaltAction & operator=( const HaltAction & );
};

//------------------------------------------------------------------------------
class InfoAction : public Action
{
  public:
    InfoAction( Target * target );
    virtual ~InfoAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    InfoAction( const InfoAction & );
    InfoAction & operator=( const InfoAction & );

  private:
};

//------------------------------------------------------------------------------
class RescueAction : public Action
{
  public:
    RescueAction( Target * target );
    virtual ~RescueAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    RescueAction( const RescueAction & );
    RescueAction & operator=( const RescueAction & );

  private:
};

//------------------------------------------------------------------------------
class AttackAction : public Action
{
  public:
    AttackAction( Target * target );
    virtual ~AttackAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    AttackAction( const AttackAction & );
    AttackAction & operator=( const AttackAction & );

  private:
};

//------------------------------------------------------------------------------
class StrikeAction : public Action
{
  public:
    StrikeAction( Target * target );
    virtual ~StrikeAction();

    virtual void init();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    virtual void update( float dt );
    virtual void render();

  protected:
    StrikeAction( const StrikeAction & );
    StrikeAction & operator=( const StrikeAction & );

  private:
};

#endif

//==============================================================================
