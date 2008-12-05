//==============================================================================

#include <sstream>
#include <cstdarg>
#include <algorithm>

#include <hge.h>
#include <hgesprite.h>
#include <hgeanim.h>
#include <hgeparticle.h>
#include <hgeresource.h>
#include <Box2D.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>

#include <engine.hpp>
#include <entity.hpp>
#include <viewport.hpp>

//------------------------------------------------------------------------------

int Entity::s_nextGroupIndex( -1 );

namespace
{
    const char *  GUY_FRAME[] =
    {
        "guy1",
        "guy2"
    };
    const char *  SWAT_FRAME[] =
    {
        "swat1",
        "swat2"
    };
    const char ** FRAME[] =
    {
        GUY_FRAME,
        SWAT_FRAME
    };
    const char * CAR[] =
    {
        "car",
        "ute"
    };
    const char * CAR_SHADOW[] =
    {
        "car_shadow",
        "ute_shadow"
    };
    const char * TYPE_NAME[] = {
        "",
        "Vehicle",
        "",
        "Building",
        "Entity",
        ""
    };
    const char * ALLEGIANCE_NAME[] = {
        "Unknown",
        "",
        "Friendly"
        "Hostile"
    };
    const char * FIRST_NAMES[] = {
        "Johnathon",
        "Peter",
        "William",
        "Paul",
        "Mark",
        "James",
        "Richard",
        "Gordon",
        "Glen",
        "Darren",
        "Charles",
        "Godfrey",
        "Jason",
        "Jack",
        "Robert",
        "Daniel",
        "Nicholas",
        "Brendan",
        "Patrick",
        "Nathan",
        "Gregory",
        "Bradley",
        "Andrew",
        "Alexander",
        "Thomas",
        "Oliver",
        "Lucas",
        "Sebastian",
        "Christopher",
        "George",
        "Archibald"
    };
    const char * LAST_NAMES[] = {
        "Cooper",
        "Smith",
        "Hunter",
        "Williams",
        "Jones",
        "Brown",
        "Black",
        "White",
        "Taylor",
        "Johnson",
        "Anderson",
        "Walker",
        "Robinson",
        "Thompson",
        "Wilson",
        "Adams",
        "Spencer"
    };
};

//==============================================================================
Entity::Entity( float scale )
    :
    ActionTaker(),
    m_scale( scale ),
    m_type( TYPE_BASE ),
    m_id( 0 ),
    m_allegiance( ALLEGIANCE_UNKNOWN ),
    m_aabb(),
    m_visible( true )
{
}

//------------------------------------------------------------------------------
Entity::~Entity()
{
}

//------------------------------------------------------------------------------
void
Entity::init()
{
    doInit();
}

//------------------------------------------------------------------------------
void
Entity::update( float dt )
{
    updateActions( dt );
    doUpdate( dt );
}

//------------------------------------------------------------------------------
void
Entity::render()
{
    if ( ! m_visible )
    {
        return;
    }
    doRender();
    renderActions();
}

//------------------------------------------------------------------------------
void
Entity::renderGui( hgeSprite * gui, int level, Entity * picked )
{
    if ( ! m_visible )
    {
        return;
    }
    b2Body * body( getBody() );
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );
    const b2AABB & aabb( getAABB() );
    ViewPort * vp( Engine::vp() );
    hgeFont * font( rm->GetFont( "dialogue" ) );
    if ( level == 1 )
    {
        if ( m_allegiance != ALLEGIANCE_ASSET && picked != this )
        {
            return;
        }
    }
    else if ( level == 2 )
    {
        if ( m_allegiance == ALLEGIANCE_UNKNOWN && picked != this )
        {
            return;
        }
    }
    if ( m_type == TYPE_BUILDING )
    {
        Building * building( static_cast< Building * >( this ) );
        if ( building->getMeta()->getOwner() != building )
        {
            return;
        }
    }
    font->SetColor( getColor() );
    char message[256];
    message[0] = '\0';
    if ( getType() == TYPE_GUY )
    {
        Guy * guy( static_cast< Guy * >( this ) );
        if ( guy->getAllegiance() == ALLEGIANCE_ASSET )
        {
            return;
        }
    }
    if ( message[0] == '\0' )
    {
        if ( picked == this )
        {
            sprintf_s( message, 255, "[Target %s]", TYPE_NAME[m_type] );
        }
        else
        {
            sprintf_s( message, 255, "[%s %s]", ALLEGIANCE_NAME[m_allegiance],
                                                TYPE_NAME[m_type] );
        }
    }
    float width( font->GetStringWidth( message ) / vp->hscale() );
    float height( font->GetHeight() / vp->vscale() );
    float offset( 1.0f / vp->vscale() );
    width += 2.0f / vp->hscale();
    height += 2.0f / vp->vscale();
    b2Vec2 position( 0.5f * ( aabb.lowerBound.x + aabb.upperBound.x ),
                     aabb.lowerBound.y - 30.0f / vp->vscale() );
    gui->SetColor( 0x44000000 );
    gui->RenderStretch( position.x - 0.5f * width, position.y - offset,
                        position.x + 0.5f * width, position.y + height );
    font->SetScale( 1.0f / vp->hscale() );
    font->printf( position.x, position.y, HGETEXT_CENTER, message );
    position.y += 15.0f / vp->vscale();
    if ( m_type == TYPE_BUILDING )
    {
        Building * building( static_cast< Building * >( this ) );
        int num( building->getMeta()->getNumOccupants() );
        if ( num > 1 )
        {
            font->printf( position.x, position.y, HGETEXT_CENTER,
                          "(%d occupants)", num );
        }
        else if ( num > 0 )
        {
            font->printf( position.x, position.y, HGETEXT_CENTER,
                          "(%d occupant)", num );
        }
    }
    if ( m_type == TYPE_CAR )
    {
        Car * car( static_cast< Car * >( this ) );
        int num( car->getNumOccupants() );
        if ( num > 1 )
        {
            font->printf( position.x, position.y, HGETEXT_CENTER,
                          "(%d occupants)", num );
        }
        else if ( num > 0 )
        {
            font->printf( position.x, position.y, HGETEXT_CENTER,
                          "(%d occupant)", num );
        }
    }
    font->SetScale( 1.0f );
}

//------------------------------------------------------------------------------
b2Body *
Entity::getBody() const
{
    return 0;
}

//------------------------------------------------------------------------------
void
Entity::setType( EntityType type )
{
    m_type = type;
}

//------------------------------------------------------------------------------
EntityType
Entity::getType()
{
    return m_type;
}

//------------------------------------------------------------------------------
void
Entity::setVisible( bool visible )
{
    m_visible = visible;
}

//------------------------------------------------------------------------------
bool
Entity::getVisible()
{
    return m_visible;
}

//------------------------------------------------------------------------------
void
Entity::setAllegiance( EntityAllegiance allegiance )
{
    m_allegiance = allegiance;
}

//------------------------------------------------------------------------------
EntityAllegiance
Entity::getAllegiance()
{
    return m_allegiance;
}

//------------------------------------------------------------------------------
void
Entity::setID( sqlite3_int64 id )
{
    m_id = id;
}

//------------------------------------------------------------------------------
sqlite3_int64
Entity::getID()
{
    return m_id;
}

//------------------------------------------------------------------------------
const char *
Entity::getAllegianceName()
{
    return ALLEGIANCE_NAME[m_allegiance];
}

//------------------------------------------------------------------------------
const char *
Entity::getTypeName()
{
    return TYPE_NAME[m_type];
}

//------------------------------------------------------------------------------
const b2AABB &
Entity::getAABB()
{
    b2Shape * shape( getBody()->GetShapeList() );
    shape->ComputeAABB( & m_aabb, getBody()->GetXForm() );
    return m_aabb;
}

//------------------------------------------------------------------------------
DWORD
Entity::getColor()
{
    switch ( m_allegiance )
    {
        case ALLEGIANCE_UNKNOWN:
        {
            return 0xFFFFFF88;
        }
        case ALLEGIANCE_ASSET:
        {
            return 0xFFEEEEFF;
        }
        case ALLEGIANCE_FRIENDLY:
        {
            return 0xFFAAFFAA;
        }
        case ALLEGIANCE_HOSTILE:
        {
            return 0xFFFFAAAA;
        }
    }
    return 0xFFFFFFFF;
}

//------------------------------------------------------------------------------
void
Entity::onActionInterrupted( Action * action )
{
}

//------------------------------------------------------------------------------
void
Entity::onActionCompleted( Action * action )
{
}

//------------------------------------------------------------------------------
//static:
//------------------------------------------------------------------------------
Entity *
Entity::factory( EntityType type )
{
    switch ( type )
    {
        case TYPE_CAR:
        {
            return new Car( 0, 0.7f );
        }
        case TYPE_TREE:
        {
            return new Tree();
        }
        case TYPE_BUILDING:
        {
            return new Building();
        }
        case TYPE_GUY:
        {
            return new Guy( 0, 0.4f );
        }
        case TYPE_PARKED:
        {
            return new Parked();
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
std::vector< Entity * >
Entity::databaseFactory( EntityType type )
{
    std::vector< Entity * > entities;
    Database database( "world.db3" );
    Query query( database );

    switch ( type )
    {
        case TYPE_CAR:
        {
            query.get_result( "SELECT id, x, y, angle, type "
                              "FROM cars" );
            break;
        }
        case TYPE_TREE:
        {
            query.get_result( "SELECT id, x, y, radius "
                              "FROM trees" );
            break;
        }
        case TYPE_BUILDING:
        {
            query.get_result( "SELECT id, x, y, angle, width, height "
                              "FROM buildings" );
            break;
        }
        case TYPE_GUY:
        {
            query.get_result( "SELECT id, x, y, angle, type "
                              "FROM guys" );
            break;
        }
        case TYPE_PARKED:
        {
            query.get_result( "SELECT id, x, y, angle, width, height "
                              "FROM parked" );
            break;
        }
    }

    while ( query.fetch_row() )
    {
        entities.push_back( factory( type ) );
        entities.back()->initFromQuery( query );    
    }

    query.free_result();

    return entities;
}

//------------------------------------------------------------------------------
int
Entity::getNextGroupIndex()
{
    return s_nextGroupIndex--;
}

//------------------------------------------------------------------------------
void
Entity::resetNextGroupIndex()
{
    s_nextGroupIndex = -1;
}

//------------------------------------------------------------------------------
//protected
//------------------------------------------------------------------------------
void
Entity::persistToDatabase( char * table, char * rows[], ... )
{
    int num( 0 );
    char * names[10];
    char * types[10];
    while ( * rows != 0 && num < 10 )
    {
        names[num] = * ( rows++ );
        types[num] = * ( rows++ );
        ++num;
    }

    std::stringstream format;
    if ( m_id == 0 )
    {
        format << "INSERT INTO " << table << " (";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << names[i];
        }
        format << ") VALUES (";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << types[i];
        }
        format << ")";
    }
    else
    {
        format << "UPDATE " << table << " SET ";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << names[i] << "=" << types[i];
        }
        format << " WHERE id=" << m_id;
    }

    va_list args;
    char sql[256];
    va_start( args, rows );
    vsprintf_s( sql, 255, format.str().c_str(), args );
    va_end( args );

    Database database( "world.db3" );
    Query query( database );

    if ( ! query.execute( sql ) )
    {
        Engine::hge()->System_Log( "Query Failed: %s", sql );
    }
    else if ( m_id == 0 )
    {
        m_id = query.insert_id();
    }
}

//------------------------------------------------------------------------------
void
Entity::deleteFromDatabase( const char * table )
{
    if ( m_id == 0 )
    {
        return;
    }
    Database database( "world.db3" );
    Query query( database );
    char sql[256];
    sprintf_s( sql, 255, "DELETE FROM %s WHERE id=%d", table, m_id );
    if ( ! query.execute( sql ) )
    {
        Engine::hge()->System_Log( "Query Failed: %s", sql );
    }
}

//==============================================================================
Damageable::Damageable( float strength )
    :
    m_max_strength( strength ),
    m_strength( strength ),
    m_damage( 0.0f ),
    m_timer( 0.0f )
{
    m_sprite = new hgeSprite( 0, 0, 0, 1, 1 );
}

//------------------------------------------------------------------------------
Damageable::~Damageable()
{
    delete m_sprite;
}

//------------------------------------------------------------------------------
void
Damageable::updateDamageable( float dt )
{
    m_timer -= dt;
    if ( m_timer < 0.0f )
    {
        m_timer = 0.0f;
    }

    if ( dt > 0.0f && m_damage > 0.0f )
    {
        float dd( m_damage * dt );
        m_strength -= dd;
        m_damage -= dd;
        if ( m_damage < 0.1f )
        {
            m_damage = 0.0f;
        }
        m_timer = 0.1f;
    }

    addStrength( 0.1f * dt );
}

//------------------------------------------------------------------------------
void
Damageable::renderDamageable( const b2Vec2 & position, float scale )
{
    if ( m_timer <= 0.0f )
    {
        return;
    }
    m_sprite->SetColor( 0xBB000000 );
    float width( 40.0f );
    float height( 4.0f );
    float x1( position.x - 0.5f * width * scale );
    float y1( position.y - 0.5f * height * scale - 20.0f * scale );
    float x2( position.x + 0.5f * width * scale );
    float y2( position.y + 0.5f * height * scale - 20.0f * scale );
    m_sprite->RenderStretch( x1, y1, x2, y2 );
    float ratio( m_strength / m_max_strength );
    m_sprite->SetColor( 0xBB000000 +
                        ( static_cast<DWORD>( ratio * 255.0f ) << 8 ) +
                        ( static_cast<DWORD>( (1.0f - ratio)*255.0f ) << 16 ) );
    x1 = position.x - 0.5f * width * scale;
    y1 = position.y - 0.5f * height * scale - 20.0f * scale;
    x2 = position.x - 0.5f * width * scale + 40.0f * ratio * scale;
    y2 = position.y + 0.5f * height * scale - 20.0f * scale;
    m_sprite->RenderStretch( x1, y1, x2, y2 );
}

//------------------------------------------------------------------------------
void
Damageable::addStrength( float amount )
{
    m_strength += amount;
    if ( m_strength > m_max_strength )
    {
        m_strength = m_max_strength;
        m_damage = 0.0f;
    }
}

//------------------------------------------------------------------------------
void
Damageable::takeDamage( float amount )
{
    if ( amount >= 0.1f )
    {
        m_damage += amount;
    }
}

//------------------------------------------------------------------------------
bool
Damageable::isDestroyed()
{
    return m_strength <= 0.0f;
}

//==============================================================================
Container::Container( int max_size )
    :
    m_max_size( max_size ),
    m_contents()
{
}

//------------------------------------------------------------------------------
Container::~Container()
{
}

//------------------------------------------------------------------------------
void
Container::updateContainer( float dt )
{
    const b2AABB & bounds( getContainerBounds() );
    b2Vec2 position( 0.5f * ( bounds.lowerBound + bounds.upperBound ) );
    HGE * hge( Engine::hge() );
    if ( m_contents.size() == 0 )
    {
        return;
    }
    std::vector< Entity * >::iterator i;
    for ( i = m_contents.begin(); i != m_contents.end(); ++i )
    {
        ( * i )->getBody()->SetXForm( position, 0.0f ); 
    }
}

//------------------------------------------------------------------------------
void
Container::enter( Entity * entity )
{
    if ( static_cast<int>( m_contents.size() ) < m_max_size &&
         allowEnter( entity ) )
    {
        entity->setVisible( false );
        entity->getBody()->GetShapeList()->m_groupIndex = getContainerGroup();
        onEnter( entity );
        m_contents.push_back( entity );
    }
}

//------------------------------------------------------------------------------
void
Container::leave( Entity * entity )
{
    std::vector< Entity * >::iterator i( std::find( m_contents.begin(),
                                                    m_contents.end(),
                                                    entity ) );
    if ( i == m_contents.end() )
    {
        return;
    }

    const b2AABB & bounds( getContainerBounds() );
    b2Vec2 position( Engine::hge()->Random_Float( bounds.lowerBound.x,
                                                  bounds.upperBound.x ),
                     Engine::hge()->Random_Float( bounds.lowerBound.y,
                                                  bounds.upperBound.y ) );
    b2Vec2 centre( 0.5f * ( bounds.lowerBound + bounds.upperBound ) );
    if ( Engine::hge()->Random_Float( 0.0f, 1.0f ) < 0.5f )
    {
        position.x = ( position.x < centre.x ) ? bounds.lowerBound.x - 2.0f
                                               : bounds.upperBound.x + 2.0f;
    }
    else
    {
        position.y = ( position.y < centre.y ) ? bounds.lowerBound.y - 2.0f
                                               : bounds.upperBound.y + 2.0f;
    }

    float angle( Engine::hge()->Random_Float( -M_PI, M_PI ) );

    m_contents.erase( i );

    entity->getBody()->SetXForm( position, angle ); 
    entity->getBody()->GetShapeList()->m_groupIndex = 0;
    entity->setVisible( true );

    onLeave( entity );
}

//------------------------------------------------------------------------------
void
Container::evacuate()
{
}

//------------------------------------------------------------------------------
int
Container::getNumOccupants()
{
    return m_contents.size();
}

//==============================================================================
Car::Car( int kind, float scale )
    :
    Entity( scale ),
    Damageable( 25.0f ),
    Container( 0 ),
    m_car( 0 ),
    m_kind( kind )
{
    setType( TYPE_CAR );
}

//------------------------------------------------------------------------------
Car::~Car()
{
    Engine::b2d()->DestroyBody( m_car );
}

//------------------------------------------------------------------------------
void
Car::collide( Entity * entity, b2ContactPoint * point )
{
    takeDamage( point->normalForce * 0.00001f );
}

//------------------------------------------------------------------------------
b2Body *
Car::getBody() const
{
    return m_car;
}

//------------------------------------------------------------------------------
void
Car::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "type", "%d", 0 };
    Entity::persistToDatabase( "cars", rows, m_car->GetPosition().x,
                                             m_car->GetPosition().y,
                                             m_car->GetAngle(),
                                             m_kind );
}

//------------------------------------------------------------------------------
void
Car::deleteFromDatabase()
{
    Entity::deleteFromDatabase( "cars" );
}

//------------------------------------------------------------------------------
bool
Car::allowEnter( Entity * entity )
{
    return true;
}

//------------------------------------------------------------------------------
int
Car::getContainerGroup()
{
    return getBody()->GetShapeList()->m_groupIndex;
}

//------------------------------------------------------------------------------
const b2AABB &
Car::getContainerBounds()
{
    return m_aabb;
}

//------------------------------------------------------------------------------
void
Car::onEnter( Entity * entity )
{
    if ( entity->getType() == TYPE_GUY )
    {
        static_cast< Guy * >( entity )->setLast( this );
    }
}

//------------------------------------------------------------------------------
void
Car::onLeave( Entity * entity )
{
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Car::doInit()
{
    if ( m_kind < 0 )
    {
        m_kind = 0;
    }
    if ( m_kind > 1 )
    {
        m_kind = 1;
    }

    m_max_size = 4 - 2 * m_kind;

    hgeResourceManager * rm( Engine::rm() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_car = Engine::b2d()->CreateDynamicBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.groupIndex = Entity::getNextGroupIndex();
    shapeDef.SetAsBox( 6.0f * m_scale, 15.0f * m_scale );
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_car->CreateShape( & shapeDef );
    m_car->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Car::doUpdate( float dt )
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );

    updateDamageable( dt );
    updateContainer( dt );

    if ( isDestroyed() )
    {
        return;
    }

    if ( Engine::instance()->isPaused() )
    {
        return;
    }
}

//------------------------------------------------------------------------------
void
Car::doRender()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( rm->GetSprite( CAR_SHADOW[m_kind] ) );
    b2Vec2 position( m_car->GetPosition() );
    float angle( m_car->GetAngle() );
    sprite->RenderEx( position.x - 1.0f, position.y - 2.0f, angle, m_scale );
    sprite = rm->GetSprite( CAR[m_kind] );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
void
Car::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_kind = static_cast< int >( query.getnum() );

    init();

    m_car->SetXForm( position, angle );
}

//==============================================================================
Guy::Guy( int kind, float scale )
    :
    Entity( scale ),
    Damageable( 25.0f ),
    m_guy( 0 ),
    m_frame( 0 ),
    m_counter( 0.0f ),
    m_kind( kind ),
    m_last( 0 )
{
    setType( TYPE_GUY );
    m_supported = static_cast< ActionType >( m_supported | TYPE_MOVE );
    m_entity = this;
    sprintf_s( m_name, 31, "%s %s",
        FIRST_NAMES[Engine::hge()->Random_Int( 0, 30)],
        LAST_NAMES[Engine::hge()->Random_Int(0, 16)] );
}

//------------------------------------------------------------------------------
Guy::~Guy()
{
    Engine::b2d()->DestroyBody( m_guy );
}

//------------------------------------------------------------------------------
void
Guy::collide( Entity * entity, b2ContactPoint * point )
{
    takeDamage( point->normalForce * 0.00001f );
    collideActions( entity, point );
}

//------------------------------------------------------------------------------
b2Body *
Guy::getBody() const
{
    return m_guy;
}

//------------------------------------------------------------------------------
void
Guy::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "type", "%d", 0 };
    Entity::persistToDatabase( "guys", rows, m_guy->GetPosition().x,
                                             m_guy->GetPosition().y,
                                             m_guy->GetAngle(),
                                             m_kind );
}

//------------------------------------------------------------------------------
void
Guy::deleteFromDatabase()
{
    Entity::deleteFromDatabase( "guys" );
}

//------------------------------------------------------------------------------
void
Guy::onActionInterrupted( Action * action )
{
    HGE * hge( Engine::hge() );
    hge->System_Log( "BREAK" );
}

//------------------------------------------------------------------------------
void
Guy::onActionCompleted( Action * action )
{
    switch ( action->getType() )
    {
        case TYPE_MOVE:
        {
            Entity * entity( action->getTarget()->getEntity() );
            if ( entity != 0 && entity->getType() == TYPE_BUILDING )
            {
                Building * building( static_cast< Building * >( entity ) );
                building->getMeta()->enter( this );
            }
            if ( entity != 0 && entity->getType() == TYPE_CAR )
            {
                Car * car( static_cast< Car * >( entity ) );
                car->enter( this );
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
const char *
Guy::getName()
{
    return m_name;
}

//------------------------------------------------------------------------------
void
Guy::setLast( Entity * last )
{
    m_last = last;
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Guy::doInit()
{
    if ( m_kind < 0 )
    {
        m_kind = 0;
    }
    if ( m_kind > 1 )
    {
        m_kind = 1;
    }
    hgeResourceManager * rm( Engine::rm() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_guy = Engine::b2d()->CreateDynamicBody( & bodyDef );
    b2CircleDef shapeDef;
    shapeDef.radius = 7.0f * m_scale;
    shapeDef.localPosition.Set( 0.0f, 0.0f );
    shapeDef.density = 0.1f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_guy->CreateShape( & shapeDef );
    m_guy->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Guy::doUpdate( float dt )
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );

    updateDamageable( dt );

    if ( isDestroyed() )
    {
        return;
    }

    if ( Engine::instance()->isPaused() )
    {
        return;
    }

    if ( ! getVisible() )
    {
        return;
    }

    switch ( m_allegiance )
    {
        case ALLEGIANCE_ASSET:
        {
            break;
        }
        case ALLEGIANCE_UNKNOWN:
        case ALLEGIANCE_FRIENDLY:
        case ALLEGIANCE_HOSTILE:
        {
            if ( ! hasAction( TYPE_MOVE ) )
            {
                _moveAtRandom();
            }
        }
    }

    float speed( m_guy->GetLinearVelocity().Length() );
    if ( speed < 0.1f )
    {
        m_frame = 0;
        m_counter = 0.0f;
    }
    else
    {
        m_counter += dt;
        if ( m_counter * speed > 1.0f )
        {
            m_frame += 1;
            if ( m_frame > 1 )
            {
                m_frame = 0;
            }
            m_counter = 0.0f;
        }
    }
}

//------------------------------------------------------------------------------
void
Guy::doRender()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( rm->GetSprite( "guy_shadow" ) );
    b2Vec2 position( m_guy->GetPosition() );
    float angle( m_guy->GetAngle() );
    sprite->Render4V( position.x-26.0f * m_scale, position.y-30.0f * m_scale,
                      position.x-8.0f * m_scale, position.y-30.0f * m_scale,
                      position.x+8.0f * m_scale, position.y+12.0f * m_scale,
                      position.x-8.0f * m_scale, position.y+12.0f * m_scale);
    sprite = rm->GetSprite( FRAME[m_kind][m_frame] );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
void
Guy::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_kind = static_cast< int >( query.getnum() );

    if ( m_kind == 1 )
    {
        m_allegiance = ALLEGIANCE_ASSET;
    }

    init();

    m_guy->SetXForm( position, angle );
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Guy::_moveAtRandom()
{
    HGE * hge( Engine::hge() );

    b2AABB aabb;
    b2Vec2 range( 100.0f, 100.0f );
    aabb.lowerBound = m_guy->GetPosition() - range;
    aabb.upperBound = m_guy->GetPosition() + range;
    b2Shape * shapes[10];
    int num( Engine::b2d()->Query( aabb, shapes, 10 ) );

    if ( num == 0 )
    {
        return;
    }

    Target * target( 0 );
    int i( hge->Random_Int( 0, num - 1 ) );
    while ( num > 0 && target == 0 )
    {
        b2Shape * shape( shapes[i] );
        if ( shape == 0 )
        {
            break;
        }
        Entity * entity( static_cast< Entity * >(
            shape->GetBody()->GetUserData() ) );
        if ( entity != this )
        {
            switch ( entity->getType() )
            {
                case TYPE_BUILDING:
                {
                    Building * building( static_cast< Building * >( entity ) );
                    Entity * owner( static_cast< Entity * >(
                                        building->getMeta()->getOwner() ) );
                    if ( m_last != owner )
                    {
                        target = new Target( entity );
                    }
                    break;
                }
                case TYPE_CAR:
                {
                    if ( m_last != entity )
                    {
                        target = new Target( entity );
                    }
                    break;
                }
            }
            break;
        }
        else
        {
            shapes[i] = 0;
        }
        ++i;
        if ( i >= num )
        {
            i = 0;
        }
    }
    if ( target == 0 )
    {
        range = m_guy->GetPosition();
        range.x += hge->Random_Float( -100.0f, 100.0f );
        range.y += hge->Random_Float( -100.0f, 100.0f );
        target = new Target( range );
        m_last = 0;
    }

    addAction( Action::factory( TYPE_MOVE, target ) );
}

//==============================================================================
Tree::Tree( float radius, float scale )
    :
    Entity( scale ),
    m_tree( 0 ),
    m_radius( radius )
{
    setType( TYPE_TREE );
}

//------------------------------------------------------------------------------
Tree::~Tree()
{
    Engine::b2d()->DestroyBody( m_tree );
}

//------------------------------------------------------------------------------
void
Tree::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
b2Body *
Tree::getBody() const
{
    return m_tree;
}

//------------------------------------------------------------------------------
void
Tree::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "radius", "%f", 0 };
    Entity::persistToDatabase( "trees", rows, m_tree->GetPosition().x,
                                              m_tree->GetPosition().y,
                                              m_radius );
}

//------------------------------------------------------------------------------
void
Tree::deleteFromDatabase()
{
    Entity::deleteFromDatabase( "trees" );
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Tree::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_tree = Engine::b2d()->CreateStaticBody( & bodyDef );
    b2CircleDef shapeDef;
    shapeDef.radius = m_radius;
    shapeDef.localPosition.Set( 0.0f, 0.0f );
    m_tree->CreateShape( & shapeDef );
}

//------------------------------------------------------------------------------
void
Tree::doUpdate( float dt )
{
}

//------------------------------------------------------------------------------
void
Tree::doRender()
{
}

//------------------------------------------------------------------------------
void
Tree::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    m_radius = static_cast< float >( query.getnum() );

    init();

    m_tree->SetXForm( position, 0.0f );
}

//==============================================================================
MetaBuilding::MetaBuilding()
    :
    Damageable( 25.0f ),
    Container( 100 ),
    m_aabb(),
    m_owner( 0 )
{
    m_aabb.lowerBound.x = 0.0f;
    m_aabb.lowerBound.y = 0.0f;
    m_aabb.upperBound.x = 0.0f;
    m_aabb.upperBound.y = 0.0f;
}

//------------------------------------------------------------------------------
MetaBuilding::~MetaBuilding()
{
}

//------------------------------------------------------------------------------
const b2AABB &
MetaBuilding::getAABB()
{
    return m_aabb;
}

//------------------------------------------------------------------------------
void
MetaBuilding::addAABB( const b2AABB & aabb )
{
    if ( m_aabb.lowerBound == m_aabb.upperBound )
    {
        m_aabb = aabb;
    }
    else
    {
        m_aabb.lowerBound.x = min( m_aabb.lowerBound.x, aabb.lowerBound.x );
        m_aabb.lowerBound.y = min( m_aabb.lowerBound.y, aabb.lowerBound.y );
        m_aabb.upperBound.x = max( m_aabb.upperBound.x, aabb.upperBound.x );
        m_aabb.upperBound.y = max( m_aabb.upperBound.y, aabb.upperBound.y );
    }
}

//------------------------------------------------------------------------------
void
MetaBuilding::setOwner( Building * building )
{
    m_owner = building;
}

//------------------------------------------------------------------------------
Building *
MetaBuilding::getOwner()
{
    return m_owner;
}

//------------------------------------------------------------------------------
void
MetaBuilding::doUpdate( float dt )
{
    updateDamageable( dt ); 
    updateContainer( dt ); 
    if ( m_contents.size() > 0 &&
         Engine::hge()->Random_Float( 0.0f, 100.0f ) < 1.0f )
    {
        leave(m_contents[Engine::hge()->Random_Int(0, m_contents.size() - 1)]);
    }
}

//------------------------------------------------------------------------------
bool
MetaBuilding::allowEnter( Entity * entity )
{
    return entity->getAllegiance() != ALLEGIANCE_ASSET;
}

//------------------------------------------------------------------------------
int
MetaBuilding::getContainerGroup()
{
    return getOwner()->getBody()->GetShapeList()->m_groupIndex;
}

//------------------------------------------------------------------------------
void
MetaBuilding::onEnter( Entity * entity )
{
    if ( entity->getType() == TYPE_GUY )
    {
        static_cast< Guy * >( entity )->setLast( getOwner() );
    }
}

//------------------------------------------------------------------------------
void
MetaBuilding::onLeave( Entity * entity )
{
}

//------------------------------------------------------------------------------
const b2AABB &
MetaBuilding::getContainerBounds()
{
    return m_aabb;
}

//==============================================================================
Building::Building( float width, float height, float scale )
    :
    Entity( scale ),
    m_building( 0 ),
    m_width( width ),
    m_height( height ),
    m_meta( 0 )
{
    setType( TYPE_BUILDING );
}

//------------------------------------------------------------------------------
Building::~Building()
{
    if ( m_meta != 0 && m_meta->getOwner() == this )
    {
        delete m_meta;
    }
    Engine::b2d()->DestroyBody( m_building );
}

//------------------------------------------------------------------------------
void
Building::amalgamate()
{
    HGE * hge( Engine::hge() );
    Entity::getAABB();
    b2Shape * shapes[16];
    int num( Engine::b2d()->Query( m_aabb, shapes, 16 ) );
    if ( num == 16 )
    {
        hge->System_Log( "Too many shape collisions for building!" );
    }
    for ( int i = 0; i < num; ++i )
    {
        if ( shapes[i] == m_building->GetShapeList() )
        {
            continue;
        }
        Entity * entity =
            static_cast< Entity * >( shapes[i]->GetBody()->GetUserData() );
        if ( entity->getType() != TYPE_BUILDING )
        {
            continue;
        }
        b2Vec2 v1;
        b2Vec2 v2;
//      float distance(b2Distance( & v1, & v2, shape, m_building->GetXForm(),
//                                 & shape[i], entity->getBody()->GetXForm() ));
//      if ( distance > 20.0f )
//      {
//          continue;
//      }
        // perform accurate collision test
        Building * building( static_cast< Building * >( entity ) );
        if ( building->getMeta() == 0 )
        {
            hge->System_Log( "Existing building has no meta!" );
        }
        if ( m_meta == 0 )
        {
            m_meta = building->getMeta();
        }
        else if ( m_meta != building->getMeta() )
        {
            building->setMeta( m_meta );
        }
    }
    if ( m_meta == 0 )
    {
        m_meta = new MetaBuilding();
        m_meta->setOwner( this );
    }
    m_meta->addAABB( m_aabb );
    getBody()->GetShapeList()->m_groupIndex =
        m_meta->getOwner()->getBody()->GetShapeList()->m_groupIndex;
}

//------------------------------------------------------------------------------
MetaBuilding *
Building::getMeta()
{
    return m_meta;
}

//------------------------------------------------------------------------------
void
Building::setMeta( MetaBuilding * meta )
{
    HGE * hge( Engine::hge() );
    if ( m_meta == 0 )
    {
        hge->System_Log( "Told to change when I have none!" );
        return;
    }
    if ( m_meta->getOwner() == this )
    {
        delete m_meta;
    }
    m_meta = meta;
    amalgamate();
}

//------------------------------------------------------------------------------
const b2AABB &
Building::getAABB()
{
    return m_meta->getAABB();
}

//------------------------------------------------------------------------------
void
Building::collide( Entity * entity, b2ContactPoint * point )
{
    m_meta->takeDamage( point->normalForce * 0.00001f );
}

//------------------------------------------------------------------------------
b2Body *
Building::getBody() const
{
    return m_building;
}

//------------------------------------------------------------------------------
void
Building::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "width", "%f",
                      "height", "%f", 0 };
    Entity::persistToDatabase( "buildings", rows, m_building->GetPosition().x,
                                                  m_building->GetPosition().y,
                                                  m_building->GetAngle(),
                                                  m_width,
                                                  m_height );
}

//------------------------------------------------------------------------------
void
Building::deleteFromDatabase()
{
    Entity::deleteFromDatabase( "buildings" );
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Building::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_building = Engine::b2d()->CreateStaticBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.groupIndex = Entity::getNextGroupIndex();
    shapeDef.SetAsBox( 0.5f * m_width * m_scale, 0.5f * m_height * m_scale );
    m_building->CreateShape( & shapeDef );
}

//------------------------------------------------------------------------------
void
Building::doUpdate( float dt )
{
    if ( m_meta->getOwner() == this )
    {
        m_meta->doUpdate( dt );
    }
}

//------------------------------------------------------------------------------
void
Building::doRender()
{
}

//------------------------------------------------------------------------------
void
Building::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_width = static_cast< float >( query.getnum() );
    m_height = static_cast< float >( query.getnum() );

    init();

    m_building->SetXForm( position, angle );

    amalgamate();
}

//==============================================================================
Parked::Parked( float width, float height, float scale )
    :
    Entity( scale ),
    m_parked( 0 ),
    m_width( width ),
    m_height( height )
{
    setType( TYPE_PARKED );
}

//------------------------------------------------------------------------------
Parked::~Parked()
{
    Engine::b2d()->DestroyBody( m_parked );
}

//------------------------------------------------------------------------------
void
Parked::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
b2Body *
Parked::getBody() const
{
    return m_parked;
}

//------------------------------------------------------------------------------
void
Parked::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "width", "%f",
                      "height", "%f", 0 };
    Entity::persistToDatabase( "parked", rows, m_parked->GetPosition().x,
                                               m_parked->GetPosition().y,
                                               m_parked->GetAngle(),
                                               m_width,
                                               m_height );
}

//------------------------------------------------------------------------------
void
Parked::deleteFromDatabase()
{
    Entity::deleteFromDatabase( "parked" );
}

//------------------------------------------------------------------------------
//protected:
//------------------------------------------------------------------------------
void
Parked::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_parked = Engine::b2d()->CreateStaticBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.SetAsBox( 0.5f * m_width * m_scale, 0.5f * m_height * m_scale );
    m_parked->CreateShape( & shapeDef );
}

//------------------------------------------------------------------------------
void
Parked::doUpdate( float dt )
{
}

//------------------------------------------------------------------------------
void
Parked::doRender()
{
}

//------------------------------------------------------------------------------
void
Parked::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_width = static_cast< float >( query.getnum() );
    m_height = static_cast< float >( query.getnum() );

    init();

    m_parked->SetXForm( position, angle );
}

//==============================================================================
