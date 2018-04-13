#ifndef BSPLINE_HPP
#define BSPLINE_HPP
#include "generalHeader.h"

class BSpline{
public:
      int orderK = 3;
      void setK(int k){
            int tempK = std::max(std::min(k,(int)controlPoints.size()),1);
            if(tempK==orderK){

            }else{
                  orderK = tempK;
                  resetKnots();
            }
      };
      vector<float> knots;
      // degree will always be orderK-1
      // continuity will always be degree-1
      float du = 0.001;
      float getKnotUi(int i); // knotU implemented as function for change
      float getStandardKnotUi(int i);

      vector<vec3> controlPoints;
      void addControlPoints(vec3 p){controlPoints.push_back(p);resetKnots();};
      int pointPixelSize = 20;
      int getm(){return controlPoints.size()-1;};
      // m will always be controlPoints.size-1
      //  Pi is indexed from 0 to m
      int delta(float u,int m, int k);
      vec3 E_delta_1(int k, int m, float u);
      void resetKnots();
      vector<vec3> getMesh();
      vector<vec3> getJoints();
      vector<vec3> getLines();
      float N(int i, int k, float u);
      BSpline(){
            resetKnots();
      }
      BSpline(vector<vec3> ps){
            controlPoints = ps;
            resetKnots();
      }

};

#endif
