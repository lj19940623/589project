#ifndef GENERALHEADER_H
#define GENERALHEADER_H
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <tuple>
#include <vector>
#include <map>
#include <string>
#include <math.h>
using namespace std;

#include <ctime>
#include <chrono>
using namespace std::chrono;

#ifdef MACOS
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#endif


// Include GLM
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"
using namespace glm;

#endif
