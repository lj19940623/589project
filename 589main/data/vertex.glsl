#version 410

layout(location = 0) in vec4 vertices;

// uniform mat4 t;
uniform mat4 m;
uniform mat4 v;
// uniform mat4 p;

void main()
{

      // vec4 world_coor = m*t*vec4(vertices.xyz,1);
      // vec4 camera_coor = v*world_coor;
      // vec4 window_coor = p*camera_coor;
      // gl_Position =  window_coor;
      gl_Position = m*v*vec4(vertices.xyz,1);

}
