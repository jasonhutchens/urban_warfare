//==============================================================================

#ifndef ArseContext
#define ArseContext

class Context
{
  public:
    Context();
    virtual ~Context();

  private:
    Context( const Context & );
    Context & operator=( const Context & );

  public:
    virtual void init() = 0;
    virtual void fini() = 0;
    virtual bool update( float dt ) = 0;
    virtual void render() = 0;
};

#endif

//==============================================================================
