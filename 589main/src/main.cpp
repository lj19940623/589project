#include "generalHeader.h"
#include "VertexArray.hpp"
#include "Program.hpp"
#include "RenderableObj.hpp"
#ifdef MACOS
#define windowI  2
#else
#define windowI  1
#endif

//------------------
// attribute
//------------------
// window
GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
int windowWidth = 600;
int windowHeight =800;
// camera atrr
// lookAt
vec3 eye = vec3(0,0,1);
vec3 center = vec3(0,0,0);
vec3 up = vec3(0,1,0);
// perspective
mat4 p;
float fovy = 60;
float aspect = (float)windowWidth / (float)windowHeight;
float zNear = 0.1f;
float zFar = 100.0f;
// camera angles
mat4 v;
float camera_distance = 1.0f;
float anglleftright = 0;
float angleupdown = 0;
// callbacks
bool rightPressed = false;
bool leftPressed = false;
bool midPressed = false;
bool shiftPressed = false;
float xcur;
float ycur;
float xlastoffset;
float ylastoffset;
//
bool printInf = false;
//
vector<renderableObj> objList;

// // time related attri
// steady_clock::time_point t1;
// steady_clock::time_point t2;


//------------------
// function shortcut
//------------------
void render(Program &program, VertexArray &va);
void renderPoints(Program &program, VertexArray &va, int pointsize);
void renderLines(Program &program, VertexArray &va);

void CheckGLErrors();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
//------------------
// main function
//------------------
int main(int argc, char *argv[]){
	// Initialise GLFW
	if( !glfwInit()){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
	cout << "GLFW inited" << endl;

	// glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	cout << "GLFW hint set" << endl;

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( windowWidth, windowHeight, "A01", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	cout << "GLFW window created" << endl;

	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	CheckGLErrors();
	cout << "Init callbacks\n";

	// Create and compile our GLSL program from the shaders
	Program program("data/vertex.glsl", "data/fragment.glsl");
	CheckGLErrors();
	cout << "Shader & Program inited\n";

	// // t m: obj on obj and obj to world, specified by each obj
	GLint t44 = glGetUniformLocation(program.id, "t");
	GLint m44 = glGetUniformLocation(program.id, "m");
	// // v p: world to camera and camera to perspective, apply to all objs
	GLint v44 = glGetUniformLocation(program.id, "v");
	GLint p44 = glGetUniformLocation(program.id, "p");
	GLint colorId = glGetUniformLocation(program.id, "lightColor");

	Server sv(2225);
	sv.start();
	cout <<"server started, please open ios app with proper ip target!\n";

	cout << "start of while loop rendering"<<endl;
	while(!glfwWindowShouldClose(window)){
		if(printInf){
			cout << "INFO:\t " ;
			// add here
			cout << "\n" ;
			}
			printInf = false;
		}
		// update window
		glUseProgram(program.id);
		// get window size and reset viewport
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		// glViewport(0,0,windowWidth,windowHeight); // normalized coor to window coor
		glViewport(0,0,windowI*windowWidth,windowI*windowHeight); // normalized coor to window coor


		glUseProgram(program.id);
		// world to camara mat
		// update v
		eye = vec3(
			camera_distance*sqrt(1.0f-sin(angleupdown)*sin(angleupdown))*sin(anglleftright),
			camera_distance*sin(angleupdown),
			camera_distance*sqrt(1.0f-sin(angleupdown)*sin(angleupdown))*cos(anglleftright));
		v = lookAt(eye+center, center, up);
		glUniformMatrix4fv(v44, 1, GL_FALSE, value_ptr(v));
		// camara to window mat
		// update p
		aspect = (float)windowWidth / (float)windowHeight;
		p = perspective(radians(fovy), aspect, zNear, zFar);
		glUniformMatrix4fv(p44, 1, GL_FALSE, value_ptr(p));

		//
		CheckGLErrors();

		// clear
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		CheckGLErrors();

		// rendering
		std::vector<vec3> v;
		v.push_back(vec3(1,0,0));
		v.push_back(vec3(0,0,0));
		v.push_back(vec3(0,1,0));
		v.push_back(vec3(0,0,0));
		v.push_back(vec3(0,0,1));
		v.push_back(vec3(0,0,0));
		VertexArray va(6);
		va.addBuffer("axis", 0, v);

		CheckGLErrors();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
	cout << "End of destroy \n ";

	return 0;
}

//------------------
// function field
//------------------

void render(Program &program, VertexArray &va) {
	glUseProgram(program.id);
	glBindVertexArray(va.id);
	// glDrawArrays(GL_LINE_STRIP, 0, va.count);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, va.count);
	glBindVertexArray(0);
	CheckGLErrors();
}
void renderPoints(Program &program, VertexArray &va, int pointsize) {
	glUseProgram(program.id);
	glBindVertexArray(va.id);
	glPointSize(pointsize);
	glDrawArrays(GL_POINTS, 0, va.count);
	glBindVertexArray(0);
	CheckGLErrors();
}
void renderLines(Program &program, VertexArray &va) {
	glUseProgram(program.id);
	glBindVertexArray(va.id);
	glDrawArrays(GL_LINES, 0, va.count);
	glBindVertexArray(0);
	CheckGLErrors();
}

void CheckGLErrors(){
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
			case GL_INVALID_ENUM: cout << "GL_INVALID_ENUM" << endl; break;
			case GL_INVALID_VALUE: cout << "GL_INVALID_VALUE" << endl; break;
			case GL_INVALID_OPERATION: cout << "GL_INVALID_OPERATION" << endl; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
			case GL_OUT_OF_MEMORY: cout << "GL_OUT_OF_MEMORY" << endl; break;
			default: cout << "[unknown error code]" << endl;
		}
	}
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	// control
	if (key == GLFW_KEY_SHIFT && action == GLFW_PRESS) shiftPressed = true;
	if (key == GLFW_KEY_SHIFT && action == GLFW_RELEASE) shiftPressed = false;
	// // print info
	if (key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT)) printInf = true;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera_distance -= yoffset;
	if (camera_distance<0.1) camera_distance = 0.1;
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		leftPressed = true;
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		leftPressed = false;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		rightPressed = true;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		rightPressed = false;
	} else if (button == GLFW_MOUSE_BUTTON_MIDDLE  && action == GLFW_PRESS) {
		midPressed = true;
	} else if (button == GLFW_MOUSE_BUTTON_MIDDLE  && action == GLFW_RELEASE) {
		midPressed = false;
	}
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{

	xlastoffset = xcur - (xcur = (float)xpos/windowWidth*2-1);	// normalized
	ylastoffset = ycur - (ycur = (float)(windowHeight-ypos)/windowHeight*2-1);
	if(midPressed&&!shiftPressed){
		angleupdown+=ylastoffset;
		anglleftright+=xlastoffset;
	}else if(midPressed&&shiftPressed){
		vec3 xpositive = normalize(cross(up,eye));
		center += xpositive*xlastoffset+normalize(cross(eye,xpositive))*ylastoffset;
	}
}
