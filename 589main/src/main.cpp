#include "generalHeader.h"
#include "VertexArray.hpp"
#include "Program.hpp"
#include "BSpline.hpp"
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
int windowHeight =600;
// // camera atrr
// lookAt
vec3 eye = vec3(0,0,1);
vec3 center = vec3(0,0,0);
vec3 up = vec3(0,1,0);
// // perspective
// mat4 p;
// float fovy = 60;
// float aspect = (float)windowWidth / (float)windowHeight;
// float zNear = 0.1f;
// float zFar = 100.0f;
// camera angles
mat4 v;
float camera_distance = 1.0f;
float anglleftright = 0;
float angleupdown = 0;
// obj
mat4 m;
float zoom=1.0;
// float rotat=1.0;
// callbacks
// mouse
bool rightPressed = false;
bool leftPressed = false;
int selected = 0;
int idx;
vector<int> idxv;
float xcur;
float ycur;
float scrollY = 2;
float key_az = 0.01;
bool revolution = false;
bool resetknots = false;
bool printInf = false;
bool multi = false;
// float xcuroffset;
// float ycuroffset;

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
	glfwSetWindowAspectRatio(window, 1, 1);
	CheckGLErrors();
	cout << "Init callbacks\n";

	// Create and compile our GLSL program from the shaders
	Program program("data/vertex.glsl", "data/fragment.glsl");
	CheckGLErrors();
	cout << "Shader & Program inited\n";


	BSpline bspline = BSpline();
	cout << "Init BSpline\n";


	// // t m: obj on obj and obj to world, specified by each obj
	// GLint t44 = glGetUniformLocation(program.id, "t");
	GLint m44 = glGetUniformLocation(program.id, "m");
	// // v p: world to camera and camera to perspective, apply to all objs
	GLint v44 = glGetUniformLocation(program.id, "v");
	// GLint p44 = glGetUniformLocation(program.id, "p");
	GLint colorId = glGetUniformLocation(program.id, "lightColor");



	cout << "start of while loop rendering"<<endl;
	while(!glfwWindowShouldClose(window)){
		if(printInf){
			cout << "k " << bspline.orderK << endl;
			cout << "num of control points " << bspline.controlPoints.size() << endl;
			cout << "m " << bspline.getm() << endl;
			cout << "num of knots " << bspline.knots.size() << endl;
			cout << "\t " ;
			for(int i=0; i<bspline.knots.size(); i++){
				cout<< bspline.knots[i]<<" ";
			}
			cout << "\n" ;
			if(bspline.controlPoints.size()>0){
				vector<vec3> tempJoints = bspline.getJoints();
				cout << "num of joints " << tempJoints.size() << endl;
				cout << "\t " ;
				for(int i=0; i<tempJoints.size(); i++){
					cout<< to_string(tempJoints[i])<<" ";
				}
				cout << "\n" ;
			}

			printInf = false;
		}
		if (resetknots){
			bspline.resetKnots();
			resetknots = false;
		}
		// control point select delete
		if (selected==1) {
			bspline.controlPoints[idx][0] = xcur;
			bspline.controlPoints[idx][1] = ycur;
			if(rightPressed){
				bspline.controlPoints.erase(bspline.controlPoints.begin()+idx);
				bspline.setK(bspline.orderK);
				bspline.resetKnots();
				selected = 0;
				leftPressed = false;
			}
		}else if(selected==2){
			for(int i=0; i<idxv.size(); i++){
				bspline.knots[idxv[i]]= (xcur+0.9)/1.8;
			}
		}else if(leftPressed){
			for(idx=0; idx<bspline.controlPoints.size(); idx++){
				if( abs((float)(bspline.controlPoints[idx][0]-xcur))<(float)bspline.pointPixelSize/windowWidth
				&& abs((float)(bspline.controlPoints[idx][1]-ycur))<(float)bspline.pointPixelSize/windowHeight
				){
					cout  << "control point # "<<idx<<" selected "<<endl;
					selected = 1;
					break;
				}
			}
			if(selected==0){
				for(idx = 0; idx < bspline.knots.size(); idx++){
					if( abs((float)(bspline.knots[idx]*1.8-0.9-xcur))<(float)bspline.pointPixelSize/windowWidth
					&& abs((float)(-0.9-ycur))<(float)bspline.pointPixelSize/windowHeight
					){
						cout  << "knots # "<<idx<<" selected "<<endl;
						idxv.push_back(idx);
						selected = 2;
						if (multi) break;// sharp
					}
				}
			}
			if(selected==0){
				bspline.addControlPoints(vec3(xcur,ycur,0));
			}
		}
		//
		glUseProgram(program.id);
		// get window size and reset viewport
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		// glViewport(0,0,windowWidth,windowHeight); // normalized coor to window coor
		glViewport(0,0,windowI*windowWidth,windowI*windowHeight); // normalized coor to window coor


		glUseProgram(program.id);
		// world to camara mat
		// update v
		eye = vec3(camera_distance*sqrt(1.0f-sin(angleupdown)*sin(angleupdown))*sin(anglleftright),
		camera_distance*sin(angleupdown),
		camera_distance*sqrt(1.0f-sin(angleupdown)*sin(angleupdown))*cos(anglleftright));
		v = lookAt(eye, center, up);
		// // camara to window
		// // update p
		// aspect = (float)windowWidth / (float)windowHeight;
		// p = perspective(radians(fovy), aspect, zNear, zFar);
		m = scale(mat4(1), vec3(zoom,zoom,zoom));
		glUniformMatrix4fv(v44, 1, GL_FALSE, value_ptr(v));
		glUniformMatrix4fv(m44, 1, GL_FALSE, value_ptr(m));
		// glUniformMatrix4fv(p44, 1, GL_FALSE, value_ptr(p));

		//
		CheckGLErrors();

		// clear
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		CheckGLErrors();

		// rendering
		//________________________
		if(bspline.controlPoints.size()>0){
			//update bspline config
			bspline.setK((int)scrollY);
			bspline.du = key_az<0?0.001:key_az;


			//draw controlPoints and geo-lines
			VertexArray va(bspline.controlPoints.size());
			va.addBuffer("controlPoints", 0, bspline.controlPoints);
			glUniformMatrix4fv(v44, 1, GL_FALSE, value_ptr(mat4(1)));
			glUniformMatrix4fv(m44, 1, GL_FALSE, value_ptr(mat4(1)));
			glUniform3f(colorId, 0.8, 0.8, 0.0);
			if(va.count>1)renderLines(program, va);
			glUniform3f(colorId, 0.1, 0.9, 0.9);
			renderPoints(program, va, bspline.pointPixelSize);
			// draw curves
			vector<vec3> linemesh = bspline.getLines();
			VertexArray vb(linemesh.size());
			vb.addBuffer("curve",0, linemesh);
			if(bspline.controlPoints.size()>1)renderLines(program, vb);
			//draw joints
			vector<vec3> joints = bspline.getJoints();
			if(joints.size()>0){
				VertexArray ve(joints.size());
				ve.addBuffer("joints",0,joints);
				glUniform3f(colorId, 0.9, 0.1, 0.9);
				renderPoints(program, ve, bspline.pointPixelSize/2);
			}
			// draw revolution
			if(revolution){
				// cout << "int revolution\n";
				vector<vec3> mesh = bspline.getMesh();
				// cout << mesh.size()<<endl;
				VertexArray vc(mesh.size());
				vc.addBuffer("revolution",0, mesh);
				m = scale(mat4(1), vec3(zoom/2,zoom/2,zoom/2));
				glUniformMatrix4fv(v44, 1, GL_FALSE, value_ptr(v));
				glUniformMatrix4fv(m44, 1, GL_FALSE, value_ptr(m));
				glUniform3f(colorId, 0.2, 0.2, 0.2);
				render(program, vc);
			}
		}
		// draw knots and basis curve
		vector<vec3> knots;
		for(int i = 0; i<=bspline.orderK+bspline.getm();i++){
			knots.push_back(vec3(bspline.getKnotUi(i)*1.8-0.9, -0.9, 0));
		}
		VertexArray vd(knots.size());
		vd.addBuffer("curve",0, knots);
		glUniform3f(colorId, 0.5, 0.1, 0.8);
		glUniformMatrix4fv(v44, 1, GL_FALSE, value_ptr(mat4(1)));
		glUniformMatrix4fv(m44, 1, GL_FALSE, value_ptr(mat4(1)));
		renderPoints(program, vd, bspline.pointPixelSize/2);
		vector<vec3> bc;
		for(int j=0; j<=bspline.getm(); j++){
			for(float i = bspline.knots[0]; i<=bspline.knots.back();i+=0.001){
				bc.push_back(vec3(i*1.8-0.9, bspline.N(j,bspline.orderK,i)/3-0.9,0));
			}
		}
		if(bc.size()>0){
			VertexArray ve(bc.size());
			ve.addBuffer("basiscurve",0, bc);
			renderLines(program, ve);
		}

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
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	// // tab change control target
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS ) {
		resetknots = true;
		camera_distance = 1.0f;
		anglleftright = 0;
		angleupdown = 0;
		zoom=1.0;
		revolution = false;
	}
	if (key == GLFW_KEY_LEFT_SHIFT  && action == GLFW_PRESS) multi = true;
	if (key == GLFW_KEY_LEFT_SHIFT  && action == GLFW_RELEASE) multi = false;
	// if (key == GLFW_KEY_LEFT_CONTROL  && action == GLFW_PRESS) ctrlPressed = true;
	// if (key == GLFW_KEY_LEFT_CONTROL  && action == GLFW_RELEASE) ctrlPressed = false;
	// // shift change control coor
	// if (key == GLFW_KEY_LEFT_SHIFT  && action == GLFW_PRESS) controlCoor = (controlCoor+1)%3;
	// if (key == GLFW_KEY_T  && (action == GLFW_PRESS || action == GLFW_REPEAT)) templen += 0.01;

	// camera control
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) angleupdown=std::max(angleupdown-0.1,-3.14/2);
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) angleupdown=std::min(angleupdown+0.1,3.14/2);
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) anglleftright+=0.1;
	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) anglleftright-=0.1;
	if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)) revolution = !revolution;
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) key_az+=0.02;
	if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) key_az-=0.02;
	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) zoom+=0.02;
	if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) zoom-=0.02;
	// if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) rotat+=0.1;
      //
	// if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT)) {if(fovy>150)fovy = 30; else fovy+=10;}
	// if (key == GLFW_KEY_KP_ADD && (action == GLFW_PRESS || action == GLFW_REPEAT)) camera_distance+=0.1;
	// if (key == GLFW_KEY_KP_SUBTRACT && (action == GLFW_PRESS || action == GLFW_REPEAT)) camera_distance-=0.1;
	// // light and tex control
	// if (key == GLFW_KEY_F1 && (action == GLFW_PRESS || action == GLFW_REPEAT)) f1 = !f1;
	// if (key == GLFW_KEY_F2 && (action == GLFW_PRESS || action == GLFW_REPEAT)) f2 = !f2;
	// if (key == GLFW_KEY_F3 && (action == GLFW_PRESS || action == GLFW_REPEAT)) f3 = !f3;
	// if (key == GLFW_KEY_F4 && (action == GLFW_PRESS || action == GLFW_REPEAT)) f4 = !f4;
	// if (key == GLFW_KEY_F5 && (action == GLFW_PRESS || action == GLFW_REPEAT)) f5 = !f5;
	// // print info
	if (key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT)) printInf = true;
	// 	printInfo();
	// }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrollY -= yoffset/4;
	if (scrollY<2) scrollY = 2;
	// cout << "K may be set "<< (int)scrollY <<endl;
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		leftPressed = true;
		// cout <<"left pressed\n";
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		leftPressed = false;
		selected = 0;
		idxv.clear();
		// cout <<"left unpressed\n";
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		rightPressed = true;
		// cout <<"right pressed\n";
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		rightPressed = false;
		// cout <<"right unpressed\n";
	}
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	xpos = (float)xpos/windowWidth*2-1;	// normalized
	ypos = (float)(windowHeight-ypos)/windowHeight*2-1;
	xcur = xpos;
	ycur = ypos;
}
