#version 410 core
out vec3 color;
uniform vec3 objColor;

// all in world
in vec3 fragEye;
in vec3 fragNormal;
in vec3 light;

void main(){
      // since obj don't have texture ao mtl. all param are hardcoded
      float ka = 0.4;
      float kd = 0.3;
      float ks = 0.3;
      float ia = 1.0;
      float ii = 1.0;
      float id = ii * clamp(dot(fragNormal, light), 0, 1);
      float gamma = 2;
      float is = ii * clamp(pow(dot(fragNormal*2.0-light, fragEye), gamma),0,1);
      float ir = ka * ia + kd * id + ks * is;
      // float ira = ka * ia;
      // // if(id<0.0)id = id*(-1.0);
      // float ird = kd * id;
      // float irs = ks * is;
      // ir = clamp(ir,0,1);
      // if(ir>=1){
      //       color = vec3((objColor * ir).x,0,0);
      // }else if(ir<=0){
      //       color = vec3(0,0,(objColor * ir).z);
      // }else{
      //       color = objColor;
      // }
      color = objColor * ir;
}
