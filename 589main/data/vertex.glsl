#version 410

layout(location = 0) in vec3 vertices;
layout(location = 1) in vec3 nor;

uniform mat4 t; // model to it self
uniform mat4 m; // model to world
uniform mat4 v; // world to view
uniform mat4 p; // view to perspective

uniform vec3 lightPos; // in world

// all in world
// out vec4 fragPosition;
out vec3 fragEye;
out vec3 fragNormal;
out vec3 light;

void main()
{
      vec4 world_coor = m*t*vec4(vertices.xyz,1);
      vec4 camera_coor = v*world_coor;
      vec4 window_coor = p*camera_coor;

      gl_Position =  window_coor;

      // prepare shading data
      // fragPosition = world_coor;
      fragEye = normalize(vec3( inverse(v)*inverse(p)*vec4(0,0,0,1))-world_coor.xyz);
      fragNormal = normalize(vec3( ( m * t * vec4(nor, 1)).xyz));
      light = normalize(lightPos - world_coor.xyz);

      // light = vec3(normalize(lightPos - vec3((m*t*vec4(vertices.xyz, 0.0)).xyz)));
      //
      // vec4 eye =   inverse(v)*vec4(0,0,0,1);        // ( v * m * t* vec4(vertices.xyz, 1.0));
      // fragEye = normalize(vec3(eye.xyz));
      // fragNormal = normalize(vec3( (v * m * t * vec4(nor, 0)).xyz));



}
