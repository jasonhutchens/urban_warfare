//==============================================================================

#ifndef ArseView
#define ArseView

#include <Box2D.h>

//------------------------------------------------------------------------------
class ViewPort
{
  public:
    ViewPort();
    ~ViewPort();
    float angle();
    b2Vec2 & offset();
    b2Vec2 & bounds();
    b2Vec2 & screen();
    void screenToWorld( b2Vec2 & point );
    float hscale() const;
    float vscale() const;
    void setAngle( float angle );

  private:
    void _updateRatios() const;

    b2Vec2 m_offset;
    b2Vec2 m_bounds;
    b2Vec2 m_screen;
    mutable float m_hscale;
    mutable float m_vscale;
    float m_angle;
};

#endif

//==============================================================================
