
#include "BSpline.hpp"


void BSpline :: resetKnots(){
      knots.clear();
      for(int i = 0; i<=orderK+getm();i++){
            knots.push_back(getStandardKnotUi(i));
      }
}
float BSpline :: getKnotUi(int i){
      return knots[i];
}
float BSpline :: getStandardKnotUi(int i){
      if(i<orderK) return 0.0;
      else if(i>getm()) return 1.0;
      else return (float)(i-orderK+1)/(getm()-orderK+2);
}
int BSpline :: delta(float u,int m, int k){
      for (int i = 0; i<=m+k-1; i++){
            if(u>=getKnotUi(i)
             && u<getKnotUi(i+1)){
                   return i;
             }
      }
      return -1;
}

vec3 BSpline :: E_delta_1(int k, int m, float u){
      int d = delta(u, m, k);
      if(d==-1){
            // cout<<"error in E_delta_1"<<endl;
            if(u==knots.front()){
                  // cout <<to_string(controlPoints.front())<<endl;
                  return controlPoints.front();
            }else{
                  // cout <<to_string(controlPoints.back())<<endl;
                  return controlPoints.back();
            }
      }
      vector<vec3> c;
      for (int i=0; i<=k-1; i++){
            c.push_back(controlPoints[d-i]);
      }
      for (int r = k; r>=2; r--){
            int i = d;
            for( int s = 0; s<=r-2; s++){
                  float omega = (u-getKnotUi(i))/(getKnotUi(i+r-1)-getKnotUi(i));
                  c[s] = c[s] * omega + c[s+1] * (1-omega);
                  i = i - 1;
            }
      }
      return c[0];
}
vector<vec3> BSpline :: getMesh(){
      vector<vec3> mesh;
      vector<vec3> linemesh = getLines();

      for(int j = 0; j<linemesh.size()-1; j++){
            for(float i = 0.0f; i<3.1415*2; i+=0.01){
                  mesh.push_back(vec3(linemesh[j][0]*cos(i),linemesh[j][1],linemesh[j][0]*sin(i)));
                  mesh.push_back(vec3(linemesh[j+1][0]*cos(i),linemesh[j+1][1],linemesh[j+1][0]*sin(i)));
            }
      }
      return mesh;
}
vector<vec3> BSpline :: getLines(){
      vector<vec3> mesh;
      for(float x = knots[orderK-1]; x <= knots[getm()+1]; x+=du){
            mesh.push_back(E_delta_1(orderK, controlPoints.size()-1, x));
            // cout <<"mesh "<<x <<" "<< to_string(E_delta_1(orderK, controlPoints.size()-1, x))<<endl;
      }
      return mesh;
}
vector<vec3> BSpline :: getJoints(){
      vector<vec3> mesh;
      if(controlPoints.size()<=1)return mesh;
      for(int i=orderK-1; i<=getm()+1; i++){
            // if(knots[i]==knots.front()||knots[i]==knots.back()){continue;}
            mesh.push_back(E_delta_1(orderK, controlPoints.size()-1, knots[i]));
      }
      return mesh;
}
float BSpline :: N(int i, int r, float u){
      if(r==1){
            if(u>=knots[i]&&u<=knots[i+1]) return 1;
            else return 0;
      } else{
            return
            ((u-knots[i])/(knots[i+r-1]-knots[i])*N(i,r-1,u)
            +(knots[i+r]-u)/(knots[i+r]-knots[i+1])*N(i+1,r-1,u));
      }
}
