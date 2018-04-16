#include "generalHeader.h"
#include "VertexArray.hpp"
#include "Program.hpp"
#include "RenderableObj.hpp"
#include "Server.cpp"
#ifdef MACOS
#define windowI  2
#else
#define windowI  1
#endif

//------------------
// attribute
//------------------
// window
GLFWwindow* window;
int windowWidth = 800;
int windowHeight = 800;
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
float anglleftright = 0.8;
float angleupdown = 0.5;
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
vector<vec3> tempPoints;
vector<vector<vec3>> stack;
vector<RenderableObj> objList;

// // time related attri
steady_clock::time_point timeLastCheck;
steady_clock::time_point timeNow;
#define getterModMax  2
int getterMod = 0;
bool spaceRelease = false;
bool spacePressed = false;
bool spaceHold = false;
bool SaveToStack = false;
#define objTypeMax 8
int TabToToggleObjType = 0;
bool makeNewObj = false;
bool readFile = false;
bool clearObjList = false;

//------------------
// function shortcut
//------------------
void render(GLenum pri, Program &program, VertexArray &va);
void renderPoints(Program &program, VertexArray &va, int pointsize);
void renderLines(Program &program, VertexArray &va);

void loadFile( std::string const & f);
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
	GLint colorId = glGetUniformLocation(program.id, "objColor");
	GLint lightPosId= glGetUniformLocation(program.id, "lightPos");

	Server sv(2225);
	sv.start();
	cout <<"server started, please open ios app with proper ip target!\n";
	RenderableObj xaxis(1);
	xaxis.v.push_back(vec3(1,0,0));
	xaxis.v.push_back(vec3(0,0,0));
	xaxis.setColor(vec3(1,0,0));
	objList.push_back(xaxis);
	RenderableObj yaxis(1);
	yaxis.v.push_back(vec3(0,1,0));
	yaxis.v.push_back(vec3(0,0,0));
	yaxis.setColor(vec3(0,1,0));
	objList.push_back(yaxis);
	RenderableObj zaxis(1);
	zaxis.v.push_back(vec3(0,0,1));
	zaxis.v.push_back(vec3(0,0,0));
	zaxis.setColor(vec3(0,0,1));
	objList.push_back(zaxis);

	cout << "start of while loop rendering"<<endl;
	while(!glfwWindowShouldClose(window)){
		if(readFile){
			string file( "./data/config.txt" );
			loadFile(file);
			readFile = false;
		}
		if(clearObjList){
			clearObjList = false;
			tempPoints.clear();
			stack.clear();
			objList.clear();
			objList.push_back(xaxis);
			objList.push_back(yaxis);
			objList.push_back(zaxis);
		}

		// getterMod == 0 => single point mode
		// getterMod == 1 => constant time interval to get point mode
		// save getter data to temp
		if(getterMod==0 && (spaceRelease == true)){
			spacePressed = false;
			cout << "get a temp point(single mode)\n";
			spaceRelease=false;
			tempPoints.push_back(sv.get3DCoor());
		}else if(getterMod==1){
			if(spacePressed && !spaceHold){
				timeLastCheck = steady_clock::now();
				spaceHold = true;
			}else if(spacePressed && spaceHold){
				steady_clock::time_point timeNow = steady_clock::now();
				duration<double> time_span = duration_cast<duration<double>>(timeNow-timeLastCheck);
				// cout << time_span.count() << "\n";
				if(time_span.count()>=0.1){ // assume we get 10 points per second
					timeLastCheck = timeNow;
					cout << "get a temp point(continues mode)\n";
					tempPoints.push_back(sv.get3DCoor());
				}
			}
		}
		// save temp data to stack
		if(SaveToStack && tempPoints.size()>0){
			SaveToStack = false;
			cout << "save "<<tempPoints.size()<<" points to stack"<<endl;
			stack.push_back(tempPoints);
			tempPoints.clear();
		}else if(SaveToStack){
			SaveToStack = false;
		}
		// save stack data to obj list
		if(makeNewObj){
			makeNewObj = false;
			RenderableObj tempObj(TabToToggleObjType, stack);
			if(tempObj.done) objList.push_back(tempObj);
		}
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

		glUniform3f(lightPosId, 5.0, 5.0, 5.0);
		//
		CheckGLErrors();

		// clear
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CheckGLErrors();

		// rendering
		// real time pen tip
		std::vector<vec3> pentip;
		vec3 v3pentip = sv.get3DCoor();
		pentip.push_back(v3pentip);
		VertexArray vpen(1);
		vpen.addBuffer("pen",0,pentip);
		renderPoints(program, vpen, 10);
		// render tempPoints and stack
		if(tempPoints.size()>0)stack.push_back(tempPoints); // push temp to stack to simplify rendering, will pop later
		int tempStackSize = stack.size();
		int colorStage = ceil(cbrt(tempStackSize+objList.size()+1)); // to make curves/objs have different color
		float colorDelta = 1.0f/(float)colorStage;
		vec3 colorCur = vec3(0.0f,0.0f,0.0f);
		if(stack.size()>0){
			for(int i = 0; i<tempStackSize; i++){
				colorCur[0] = (i%colorStage)*colorDelta;		// TODO: validate color of lines
				colorCur[1] = (i/colorStage%colorStage)*colorDelta;
				colorCur[2] = (i/colorStage/colorStage%colorStage)*colorDelta;
		            VertexArray temp(stack[i].size());
		            temp.addBuffer("temp",0,stack[i]);
				glUniform3f(colorId, colorCur.x, colorCur.y, colorCur.z);
				render(GL_LINE_STRIP, program, temp);
			}
		}
		if(tempPoints.size()>0) stack.pop_back(); // pop tempPoints
		// renderable Objs
		for(int i = 0; i<objList.size(); i++){
			// transformation
			glUniformMatrix4fv(t44, 1, GL_FALSE, value_ptr(objList[i].t));
			glUniformMatrix4fv(m44, 1, GL_FALSE, value_ptr(objList[i].m));
			// color
			if(objList[i].colored){
				glUniform3f(colorId, objList[i].color.x, objList[i].color.y, objList[i].color.z);
			}else{ // give a color
				colorCur[0] = ((i+tempStackSize)%colorStage)*colorDelta;	// TODO: validate color of objs
				colorCur[1] = ((i+tempStackSize)/colorStage%colorStage)*colorDelta;
				colorCur[2] = ((i+tempStackSize)/colorStage/colorStage%colorStage)*colorDelta;
				glUniform3f(colorId, colorCur.x, colorCur.y, colorCur.z);
			}
			// prepare vertex
			VertexArray temp(objList[i].v.size());
	            temp.addBuffer("tempV",0,objList[i].v);
	            temp.addBuffer("tempN",1,objList[i].generateNor());
			// prepare normal for shading
			render(objList[i].primitive, program, temp);
		}
		CheckGLErrors();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// print info
		if(printInf){
			cout << endl;
			cout << "INFO:\n";
			cout << "camera: " << camera_distance << " " << angleupdown<<" " << anglleftright<<" "<<endl;
			cout << "real time pentip = " << to_string(v3pentip)<<endl;
			cout << "tempPoints size = " << tempPoints.size() <<endl;
			cout << "stack size = " << stack.size() <<endl;
			cout << "objList size = " << objList.size() <<endl;
			cout << "colorStage = " << colorStage << " for " << stack.size()+(tempPoints.size()>0?1:0) << " point-data-group(temp and stack)\n";
			cout << "next obj type "<< TabToToggleObjType <<endl;
			cout << endl;
			printInf = false;
		}
	}
	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
	cout << "End of destroy \n";

	return 0;
}

//------------------
// function field
//------------------

void render(GLenum pri, Program &program, VertexArray &va) {
	glUseProgram(program.id);
	glBindVertexArray(va.id);
	glDrawArrays(pri, 0, va.count);
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
	glDrawArrays(GL_LINE_STRIP, 0, va.count);
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

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) getterMod = (getterMod+1)%getterModMax;
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) spacePressed = true;
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {spaceRelease = true;spacePressed = false;}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) SaveToStack = true;
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		TabToToggleObjType = (TabToToggleObjType+1)%objTypeMax;
		switch (TabToToggleObjType) {
			case 0:{
				cout << "objType change to (0)points\n";
				break;
			}
			case 1:{
				cout << "objType change to (1)line segments\n";
				break;
			}
			case 2:{
				cout << "objType change to (2)bspline with some default setting\n";
				break;
			}
			case 3:{
				cout << "objType change to (3)bilinear blending surface with bspline edges, min # stack = 4\n";
				break;
			}
			case 4:{
				cout << "objType change to (4)rotational blending surface with bspline strokes, min # stack = 2\n";
				break;
			}
			case 5:{
				cout << "objType change to (5)cross sectional blending surface with bspline strokes,  min # stack = 3\n";
				break;
			}
			case 6:{
				cout << "objType change to (6)spray along with bspline curve,  min # stack = 1\n";
				break;
			}
			case 7:{
				cout << "objType change to (7)sweep surface along with bspline curve,  min # stack = 2\n";
				break;
			}
		}
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS) makeNewObj = true;
	if (key == GLFW_KEY_F && action == GLFW_PRESS) readFile = true;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) clearObjList = true;
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		camera_distance = 1.0f;
		anglleftright = 0.8;
		angleupdown = 0.5;
	}
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) angleupdown += 0.05;
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) angleupdown -= 0.05;
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) anglleftright += 0.05;
	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) anglleftright -= 0.05;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	// control
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) shiftPressed = true;
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) shiftPressed = false;
	// // print info
	if (key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT)) printInf = true;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera_distance -= yoffset;
	if (camera_distance<0.2) camera_distance = 0.2;
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
		center += xpositive*xlastoffset*camera_distance+normalize(cross(eye,xpositive))*ylastoffset*camera_distance;
	}
}

void loadFile( std::string const & f) {
	if ( f.empty() ) cerr << "ERROR: Given a blank filename." << endl;
	ifstream file;
	file.open( f, fstream::in );
	if ( file.fail() )cerr << "ERROR: Couldn't load \"" << f << "\"." << endl;
	string buffer;
	vector<vec3> tempStack;
	while( file.good() ) {	// while we have data, read in a line
		buffer.clear();
		getline( file, buffer );
		size_t commentPos = buffer.find("#");	// clear out any comments
		if ( commentPos == 0 )	continue;	// the entire line was a comment? trash it!
		else if ( commentPos != string::npos ) buffer = buffer.substr( 0, commentPos ); // cut after
		else if ( buffer.size() < 1 ) break; // end of Read
		switch (buffer.at(0)) {
			// case 'T':{
			// 	float x1,y1,z1;
			// 	istringstream ss(buffer.substr(2));
			// 	ss >> x1 >> y1 >> z1;
			// 	tempPoints.push_back(vec3(x1,y1,z1));
			// 	break;
			// }
			case 'S':{
				float x1,y1,z1;
				istringstream ss(buffer.substr(2));
				ss >> x1 >> y1 >> z1;
				tempStack.push_back(vec3(x1,y1,z1));
				break;
			}
			case 'P':{
				if(tempStack.size()!=0){
					stack.push_back(tempStack);
					tempStack.clear();
				}else if(tempPoints.size()!=0){
					stack.push_back(tempPoints);
					tempPoints.clear();
				}
				break;
			}
			case 'O':{
				istringstream ss(buffer.substr(1));
				ss >> TabToToggleObjType;
				break;
			}
			case 'N':{
				RenderableObj tempObj(TabToToggleObjType, stack);
				if(tempObj.done) objList.push_back(tempObj);
				break;
			}
			case 'C':{
				float r,g,b;
				istringstream ss(buffer.substr(2));
				ss >> r >> g >> b;
				objList.back().setColor(vec3(r,g,b));
				break;
			}
			case 'T':{
				float xt, yt, zt, xr, yr, zr, xs, ys, zs;
				istringstream ss(buffer.substr(2));
				ss >> xt >> yt >> zt >> xr >> yr >> zr >> xs >> ys >> zs;
				objList.back().m = translate(mat4(1), vec3(xt,yt,zt))
				* rotate(mat4(1), zr, vec3(0,0,1)) *rotate(mat4(1), yr, vec3(0,1,0))
				* rotate(mat4(1), xr, vec3(1,0,0)) * scale(mat4(1), vec3(xs,ys,zs));
				break;
			}
			case 'D':{
				objList.push_back(objList.back());
				break;
			}
		}
	}
	file.close();
	cout << "data from file loaded, u can edit file again now.\n";
}
