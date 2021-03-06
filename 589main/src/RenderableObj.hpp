#ifndef ROBJ_HPP
#define ROBJ_HPP

#include "generalHeader.h"
#include "BSpline.hpp"
// tool kit
// 0: points
// 1: line segment
// 2: BSpline
// 3: bilinear ..
// 4: rotational ..
// 5: cross sectional ..
// 6: spray along curve
// 7: sweep surface
class RenderableObj{
      vector<vector<vec3>> savedStackData; // like control points, vertex....
public:
      bool done = false;
      bool colored = false;
      vec3 color;
      void setColor(vec3 co){
            colored = true;
            color = co;
      }
      mat4 t = mat4(1);
      mat4 m = mat4(1);
      int type;
      GLenum primitive; // pri type for rendering
      vector<vec3> v; // mesh verties for rendering
      vector<vec3> n;
      vector<vec3> generateNor(){
            if(v.size()==n.size()){
                  return n;
            }else{
                  if(primitive == GL_TRIANGLES){
                        for(int i=0; i<v.size()/3; i++){
                              vec3 nor = normalize(cross(v[3*i+2]-v[3*i+1],v[3*i+2]-v[3*i+0]));
                              n.push_back(nor);
                              n.push_back(nor);
                              n.push_back(nor);
                        }
                        if(v.size()!=n.size()) cout << "error in generateNor1\n";
                        return n;
                  }else if(primitive == GL_TRIANGLE_STRIP){
                        vec3 nor;
                        for(int i=0; i<v.size()-2; i++){
                              if(i%2==0){
                                    nor = normalize(cross(v[i+2]-v[i+1],v[i+2]-v[i+0]));
                              }else{
                                    nor = normalize(cross(v[i+2]-v[i+1],v[i+2]-v[i+0]))*(-1.0f);
                              }
                              n.push_back(nor);
                        }
                        n.push_back(nor);
                        n.push_back(nor);
                        if(v.size()!=n.size()) cout << "error in generateNor2\n";
                        return n;
                  }else{
                        vec3 nor = normalize(vec3(1,0,0));
                        for(int i=0; i<v.size(); i++){
                              n.push_back(nor);
                        }
                        if(v.size()!=n.size()) cout << "error in generateNor3\n";
                        return n;
                  }
            }
      }
      RenderableObj(int objType){ // basic constructor
            type = objType;
            v.clear();
            if(objType==1){
                  primitive = GL_LINE_STRIP;
                  done = true;
            }
      };
      RenderableObj(int objType, vector<vector<vec3>> & stack){
            type = objType;
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
                              u++;
                              uu = (float)u/maxIdxUV;
                              vec3 Qu1v = savedStackData[6][v]*(1.0f-uu) + savedStackData[7][v]*uu
                                    + savedStackData[4][u]*(1.0f-vv) + savedStackData[5][u]*vv
                                    - (savedStackData[4][0]*(1-uu)*(1-vv) + savedStackData[4][maxIdxUV]*uu*(1-vv)
                                    + savedStackData[5][0]*(1-uu)*vv + savedStackData[5][maxIdxUV]*uu*vv);
                              u--;
                              if(u%2==0){
                                    this->v.push_back(Quv);
                                    this->v.push_back(Qu1v);
                              }else{
                                    this->v.push_back(Qu1v);
                                    this->v.push_back(Quv);
                              }
                        }
                  }
                  stack.pop_back();
                  stack.pop_back();
                  stack.pop_back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 4 && stack.size()>=2){
                  // rotational blending surface
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_TRIANGLE_STRIP;
                  savedStackData.push_back(stack[stack.size()-2]); // ql
                  savedStackData.push_back(stack[stack.size()-1]); // qr
                  // z axis data will be discarded since ql qr are co-planner 2D curves (strokes)
                  for(int i=0; i<savedStackData[0].size(); i++){
                        savedStackData[0][i][2]=0;
                        savedStackData[1][i][2]=0;
                  }
                  // default bspline strokes
                  BSpline ql(savedStackData[0]);
                  BSpline qr(savedStackData[1]);
                  vector<vec3> qlLines =  ql.getLines();
                  vector<vec3> qrLines =  qr.getLines();
                  // center curve
                  vector<vec3> cu;
                  for(int i=0; i<qlLines.size(); i++){
                        cu.push_back((qlLines[i]+qrLines[i])*0.5f);
                  }
                  savedStackData.push_back(qlLines); // idx = 2
                  savedStackData.push_back(qrLines); // 3
                  savedStackData.push_back(cu); // idx = 4 , cu
                  // idea:
                  // tu(v), for fixed u, parameterizes the circle perpendicular to } with the center c(u)
                  // and passing through ql(u) and qr(u) at each u
                  int maxIdxU = 1.0f/ql.du;
                  int maxIdxV = 100;
                  float dv = 3.1415*2/(float)maxIdxV;
                  for(int u=0; u<maxIdxU-1; u++){
                        vec3 tangent1 = normalize(cu[u+1]-cu[u]); 
                        vec3 tangent2 = normalize(cu[u+2]-cu[u+1]);
                        vec4 d1 = vec4(qlLines[u]-cu[u],1);
                        vec4 d2 = vec4(qlLines[u+1]-cu[u+1],1);
                        for(int v=0; v<maxIdxV; v++){
                              this->v.push_back(vec3(rotate(dv*v, tangent1)*d1)+cu[u]);
                              this->v.push_back(vec3(rotate(dv*v, tangent2)*d2)+cu[u+1]);
                        }
                  }
                  stack.pop_back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 5 && stack.size()>=3){
                  // cross sectional blending surface
                  // idea:
                  //  move to origin -> adjust orientation -> scale -> rotate 90 degree -> move to strokes
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_TRIANGLE_STRIP;
                  savedStackData.push_back(stack[stack.size()-3]); // ql
                  savedStackData.push_back(stack[stack.size()-2]); // qr
                  savedStackData.push_back(stack[stack.size()-1]); // t: cross section
                  // interpolate 1st point of ql qr to connect them.
                  savedStackData[0][0] = savedStackData[1][0] =(savedStackData[0][0] + savedStackData[1][0])*0.5f;
                  // default bspline strokes
                  BSpline ql(savedStackData[0]);
                  BSpline qr(savedStackData[1]);
                  // clear z of tCurve for cross section
                  // move to ori for further transformation
                  vec3 save1stPointOfT = savedStackData[2][0];
                  for(int i=0; i<savedStackData[2].size(); i++){
                        savedStackData[2][i] = savedStackData[2][i] - save1stPointOfT;
                        savedStackData[2][i][2] = 0;
                  }
                  BSpline t(savedStackData[2]);
                  vector<vec3> qlLines =  ql.getLines();
                  vector<vec3> qrLines =  qr.getLines();
                  vector<vec3> tLines =  t.getLines();
                  float tLen = length(tLines.back()-tLines.front());
                  vec3 direction =normalize(tLines.back()-tLines.front());
                  vec3 directionFront = cross(direction, vec3(0,1,0));
                  vec3 directionUp = cross(directionFront, direction);
                  mat4 toOri = inverse(mat4(vec4(direction,0),vec4(directionFront,0),vec4(directionUp,0),vec4(0,0,0,1)));
                  int maxIdxU = 1.0f/ql.du;
                  for(int u=0; u<maxIdxU-1; u++){
                        vec3 direction1 =normalize(qrLines[u]-qlLines[u]);
                        vec3 direction2 =normalize(qrLines[u+1]-qlLines[u+1]);
                        vec3 tangent1 = normalize(qlLines[u+1]+qrLines[u+1]-qlLines[u]-qrLines[u]); // use as front
                        vec3 tangent2 = normalize(qlLines[u+2]+qrLines[u+2]-qlLines[u+1]-qrLines[u+1]);
                        vec3 directionUp1 = cross(tangent1,direction1);
                        vec3 directionUp2 = cross(tangent2,direction2);
                        mat4 trans1 = translate(mat4(1), qlLines[u]);
                        mat4 trans2 = translate(mat4(1), qlLines[u+1]);
                        float s1 = length(qrLines[u]-qlLines[u])/tLen;
                        float s2 = length(qrLines[u+1]-qlLines[u+1])/tLen;
                        mat4 scale1 = scale(mat4(1),vec3(s1,s1,s1));
                        mat4 scale2 = scale(mat4(1),vec3(s2,s2,s2));
                        mat4 toRLAxis1 = trans1*scale1*mat4(vec4(direction1,0),vec4(tangent1,0),vec4(directionUp1,0),vec4(0,0,0,1));
                        mat4 toRLAxis2 = trans2*scale2*mat4(vec4(direction2,0),vec4(tangent2,0),vec4(directionUp2,0),vec4(0,0,0,1));

                        for(int v = (u%2==0?0:tLines.size()-1); v < tLines.size() && v >= 0; (u%2==0?v++:v--)){
                        // for(int v=0; v<tLines.size(); v++){
                              if(u%2==0){
                                    this->v.push_back(vec3(toRLAxis1*toOri*vec4(tLines[v],1)));
                                    this->v.push_back(vec3(toRLAxis2*toOri*vec4(tLines[v],1)));
                              }else{
                                    this->v.push_back(vec3(toRLAxis2*toOri*vec4(tLines[v],1)));
                                    this->v.push_back(vec3(toRLAxis1*toOri*vec4(tLines[v],1)));
                              }
                        }
                  }
                  stack.pop_back();
                  stack.pop_back();
                  stack.pop_back();
                  done = true;
            }else if(objType == 6 && stack.size()>=1){
                  // spray
                  // idea:
                  // use GL_TRIANGLES to simulate points in cloud
                  // size is fix
                  // position, normal of triangle are random
                  float pSize = 0.008;
                  float spraySize = 0.06;
                  //
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_TRIANGLES;
                  savedStackData.push_back(stack[stack.size()-1]);
                  // default bspline strokes
                  BSpline curve(savedStackData[0]);
                  vector<vec3> cLines = curve.getLines();
                  for(int u=0; u<cLines.size()-1; u++){
                        vec3 tangent1 = normalize(cLines[u+1]-cLines[u]);
                        vec3 tangent2 = normalize(cLines[u+2]-cLines[u+1]);
                        vec3 nor = normalize(tangent2-tangent1);
                        int numOfTri = rand()%10;
                        for(int i=0; i<numOfTri; i++){
                              float r0 = (float)(rand()%628)/100;
                              vec3 position = cLines[u]+vec3(rotate(r0, tangent1)*vec4(nor,1))*spraySize*(float)(rand()%100)/100;
                              float r1 = (float)(rand()%628)/100;
                              float r2 = (float)(rand()%628)/100;
                              vec3 triNor = vec3(sqrt(1.0f-sin(r1)*sin(r1))*sin(r2),sin(r1),sqrt(1.0f-sin(r1)*sin(r1))*cos(r2));
                              v.push_back(position);
                              v.push_back(triNor*pSize+position);
                              float r3 = (float)(rand()%628)/100;
                              v.push_back(vec3(rotate(r3, triNor)*vec4(cross(triNor,vec3(0,1,0)),1))*pSize+position);
                        }
                  }
                  stack.pop_back();
                  done = true;
            }else if(objType == 7 && stack.size()>=2){
                  // sweep surface
                  v.clear();
                  savedStackData.clear();
                  primitive = GL_TRIANGLE_STRIP;
                  savedStackData.push_back(stack[stack.size()-2]);
                  savedStackData.push_back(stack[stack.size()-1]);
                  savedStackData[0].push_back(savedStackData[0][0]);
                  // discard y data and center to mid
                  vec3 avg = vec3(0,0,0);
                  for(int i=0; i<savedStackData[0].size(); i++){
                        savedStackData[0][i][1] = 0;
                        avg += savedStackData[0][i];
                  }
                  avg = avg*(1.0f/savedStackData[0].size());
                  for(int i=0; i<savedStackData[0].size(); i++){
                        savedStackData[0][i] -= avg;
                  }
                  BSpline cv(savedStackData[0]);
                  BSpline tu(savedStackData[1]);
                  tu.setK(6);
                  vector<vec3> tuLines = tu.getLines();
                  vector<vec3> cvLines = cv.getLines();
                  int maxIdxUV = 1.0f/cv.du;
                  for(int u=0; u<maxIdxUV-2; u++){
                        vec3 tangent1 = normalize(tuLines[u+1]-tuLines[u]);
                        vec3 tangent2 = normalize(tuLines[u+2]-tuLines[u+1]);
                        vec3 tangent3 = normalize(tuLines[u+3]-tuLines[u+2]);
                        vec3 nor1 = normalize(tangent2-tangent1);
                        vec3 nor2 = normalize(tangent3-tangent2);
                        vec3 binor1 = cross(tangent1,nor1);
                        vec3 binor2 = cross(tangent2,nor2);
                        mat4 trans1 = translate(mat4(1), tuLines[u]);
                        mat4 trans2 = translate(mat4(1), tuLines[u+1]);
                        mat4 toCoor1 = trans1*mat4(vec4(binor1,0),vec4(tangent1,0),vec4(nor1,0),vec4(0,0,0,1));
                        mat4 toCoor2 = trans2*mat4(vec4(binor2,0),vec4(tangent2,0),vec4(nor2,0),vec4(0,0,0,1));
                        for(int v=0; v<maxIdxUV; v++){
                              this->v.push_back(vec3(toCoor1*vec4(cvLines[v],1)));
                              this->v.push_back(vec3(toCoor2*vec4(cvLines[v],1)));
                        }
                  }
                  stack.pop_back();
                  stack.pop_back();
                  done = true;
            }
      };
};
#endif
