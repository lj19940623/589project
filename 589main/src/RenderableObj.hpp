#ifndef ROBJ_HPP
#define ROBJ_HPP

#include "generalHeader.h"
// tool kit super class
class RenderableObj{
public:
      vec3 color;
      GLenum primitive;
      RenderableObj(GLenum pri){//:primitive{pri}{
            primitive = pri;
            v.clear();
      };
      RenderableObj(GLenum pri, vec3 colorsIn, vector<vec3> vIn){//:primitive{pri}{
            primitive = pri;
            v.clear();
            color =  colorsIn;
            v = vIn;
      };
      vector<vec3> v;
};
#endif
