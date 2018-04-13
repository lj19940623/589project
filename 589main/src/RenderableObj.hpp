#ifndef ROBJ_HPP
#define ROBJ_HPP

#include "generalHeader.h"
#include "BSpline.hpp"
// tool kit
// 0: points
// 1: line segment
// 2: BSpline
// 2: ..
class RenderableObj{
      vector<vector<vec3>> savedStackData; // like control points, vertex....
public:
      GLenum primitive;
      bool done = false;
      vector<vec3> v;
      RenderableObj(int objType){//:primitive{pri}{
            v.clear();
            if(objType==1){
                  primitive = GL_LINE_STRIP;
                  done = true;
            }
      };
      RenderableObj(int objType, vector<vector<vec3>> & stack){
            if(objType == 0 && stack.size()>=1){
                  // points
                  v.clear();
                  primitive = GL_POINTS;
                  v = stack.back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 1 && stack.size()>=1){
                  // line segment
                  v.clear();
                  primitive = GL_LINE_STRIP;
                  v = stack.back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 2 && stack.size()>=1){
                  // bspline, standard knots, 3nd order
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_LINE_STRIP;
                  savedStackData.push_back(stack.back());
                  BSpline bspline(savedStackData[0]);       // BSpline class is adapted from my assignment
                  v = bspline.getLines();
                  stack.pop_back();
                  done = true;
            }else if(objType == 3 && stack.size()>=4){
                  // bilinearly blended surface
                  // use stack[stack.size()-4], stack[stack.size()-3], stack[stack.size()-2], stack[stack.size()-1]
                  // as P0(u), P1(u), Q0(v), and Q1(v)
                  // since it is hard to draw P0(0)=Q0(0)..., P0(0) Q0(0)...will be interpolated
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_TRIANGLE_STRIP;
                  savedStackData.push_back(stack[stack.size()-4]); // P0
                  savedStackData.push_back(stack[stack.size()-3]); // P1
                  savedStackData.push_back(stack[stack.size()-2]); // Q0
                  savedStackData.push_back(stack[stack.size()-1]); // Q1
                  // interpolate joint point
                  vec3 ss = (savedStackData[0][0]+savedStackData[2][0])*0.5f;
                  vec3 ee = (savedStackData[1].back()+savedStackData[3].back())*0.5f;
                  vec3 es = (savedStackData[0].back()+savedStackData[3][0])*0.5f;
                  vec3 se = (savedStackData[1][0]+savedStackData[2].back())*0.5f;
                  savedStackData[0][0] = savedStackData[2][0] = ss;
                  savedStackData[1].back() = savedStackData[3].back() = ee;
                  savedStackData[0].back() = savedStackData[3][0] = es;
                  savedStackData[1][0] = savedStackData[2].back() = se;
                  // default bspline edge curve
                  BSpline p0(savedStackData[0]);
                  BSpline p1(savedStackData[1]);
                  BSpline q0(savedStackData[2]);
                  BSpline q1(savedStackData[3]);
                  savedStackData.push_back(p0.getLines()); // idx = 4
                  savedStackData.push_back(p1.getLines()); // 5
                  savedStackData.push_back(q0.getLines()); // 6
                  savedStackData.push_back(q1.getLines()); // idx = 7
                  // make surface mesh by bilinearly blended surface formula
                  int maxIdxUV = 1.0f/p0.du;
                  for(int u = 0; u < maxIdxUV; u++){
                        for(int v = (u%2==0?0:maxIdxUV); v <= maxIdxUV && v >= 0; (u%2==0?v++:v--)){
                              float uu = (float)u/maxIdxUV;
                              float vv = (float)v/maxIdxUV;
                              vec3 Quv = savedStackData[6][v]*(1.0f-uu) + savedStackData[7][v]*uu
                                    + savedStackData[4][u]*(1.0f-vv) + savedStackData[5][u]*vv
                                    - (savedStackData[4][0]*(1-uu)*(1-vv) + savedStackData[4][maxIdxUV]*uu*(1-vv)
                                    + savedStackData[5][0]*(1-uu)*vv + savedStackData[5][maxIdxUV]*uu*vv);
                              this->v.push_back(Quv);
                              u++;
                              uu = (float)u/maxIdxUV;
                              vec3 Qu1v = savedStackData[6][v]*(1.0f-uu) + savedStackData[7][v]*uu
                                    + savedStackData[4][u]*(1.0f-vv) + savedStackData[5][u]*vv
                                    - (savedStackData[4][0]*(1-uu)*(1-vv) + savedStackData[4][maxIdxUV]*uu*(1-vv)
                                    + savedStackData[5][0]*(1-uu)*vv + savedStackData[5][maxIdxUV]*uu*vv);
                              this->v.push_back(Qu1v);
                              u--;
                        }
                  }
                  stack.pop_back();
                  stack.pop_back();
                  stack.pop_back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 1 && stack.size()>=1){
                  // line segment
                  v.clear();
                  primitive = GL_LINE_STRIP;
                  v = stack.back();
                  stack.pop_back();
                  done = true;
            }
      };
};
#endif
