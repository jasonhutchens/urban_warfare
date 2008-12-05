//==============================================================================

#ifndef ArseSplash
#define ArseSplash

#include <hge.h>

#include <context.hpp>

class hgeDistortionMesh;

//------------------------------------------------------------------------------
class Splash : public Context
{
  public:
    Splash();
    virtual ~Splash();

  private:
    Splash( const Splash & );
    Splash & operator=( const Splash & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    bool _onTime( float time );
    void _fade( float start_in, float start_out, float end_in, float end_out,
                const char * name, float scale = 0.5f );

  private:
    HCHANNEL m_channel;
    float m_timer;
    float m_delta_time;
};

#endif

//==============================================================================
