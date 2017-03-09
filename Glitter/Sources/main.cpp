// Local Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext.hpp"                           // to print vectors and matrices

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

// define macros for convenience
#define RED 1.0f, 0.0f, 0.0f
#define BLUE 0.0f, 0.0f, 1.0f
#define GREEN 0.0f, 1.0f, 0.0f

//Window resolution
#define WIDTH 800.0f
#define HEIGHT 600.0f

const GLchar* vertexSource =
"#version 150 core\n"                                                 // glsl version
"in vec3 position;"                                                   // expects 3 values for position
"in vec3 color;"                                                      // and 3 values for color
"out vec3 Color;"
"uniform mat4 projection;"
"uniform mat4 view;"                                                  // will pass color along pipeline
"uniform mat4 model;"                                                 // uniform = the same for all vertices
"void main()"
"{"
"    Color = color;"                                                  // just pass color along without modifying it
"    gl_Position = projection * view * model * vec4(position, 1.0);"  // gl_Position is special variable for final position
"}";                                                                  // must be in homogeneous coordinates -- put in 0 for z and 1 for w

// multiply by model matrix to transform
const GLchar* fragmentSource =
"#version 150 core\n"
"in vec3 Color;"
"out vec4 outColor;"
"void main()"
"{"
"    outColor = vec4(Color, 1.0);"
"}";

// vertex data
GLfloat vertices [] = {
  //Drawing a Cube; Redid my model due to the top and bottom
  //not working correctly for some reason
  //Front
  //Triangle 1
   0.5f,  0.5f, 0.5f,  RED,     // Top Right
   0.5f, -0.5f, 0.5f,  BLUE,    // Bottom Right
  -0.5f,  0.5f, 0.5f,  GREEN,   // Top Left
  //Triangle 2
   0.5f, -0.5f, 0.5f,  RED,     // Bottom Right
  -0.5f, -0.5f, 0.5f,  BLUE,    // Bottom Left
  -0.5f,  0.5f, 0.5f,  GREEN,   // Top Left

  //Back
  //Triangle 1
   0.5f,  0.5f, -0.5f, RED,     // Top Right
   0.5f, -0.5f, -0.5f, BLUE,    // Bottom Right
  -0.5f,  0.5f, -0.5f, GREEN,   // Top Left
  //Triangle 2
   0.5f, -0.5f, -0.5f, RED,     // Bottom Right
  -0.5f, -0.5f, -0.5f, BLUE,    // Bottom Left
  -0.5f,  0.5f, -0.5f, GREEN,   // Top Left

  //Right Side
  //Triangle 1
  0.5f,  0.5f, -0.5f,  RED,     // Top Right (back)
  0.5f, -0.5f, -0.5f,  BLUE,    // Bottom Right (back)
  0.5f,  0.5f,  0.5f,  GREEN,   // Top Right (Front)
  //Triangle 2
  0.5f, -0.5f,  0.5f,  RED,     //Bottom Right (front)
  0.5f,  0.5f,  0.5f,  BLUE,    //Top Right (front)
  0.5f, -0.5f, -0.5f,  GREEN,   //Bottom Right (back)

  //Left Side
  //Triangle 1
  -0.5f,  0.5f, -0.5f, RED,     // Top Left (back)
  -0.5f, -0.5f, -0.5f, BLUE,    // Bottom Left (back)
  -0.5f,  0.5f,  0.5f, GREEN,   // Top Left (Front)
  //Triangle 2
  -0.5f, -0.5f,  0.5f, RED,     //Bottom Left (front)
  -0.5f,  0.5f,  0.5f, BLUE,    //Top Left (front)
  -0.5f, -0.5f, -0.5f, GREEN,   //Bottom Left (back)

  //Top
  //Triangle 1
   0.5f,  0.5f, -0.5f, RED,     // Top Right (back)
   0.5f,  0.5f,  0.5f, BLUE,    // Top Right (front)
  -0.5f,  0.5f, -0.5f, GREEN,   // Top Left (back)
  //Triangle 2
  -0.5f, 0.5f,  0.5f,  RED,      //Top Left (front)
  -0.5f, 0.5f, -0.5f,  BLUE,     //Top Left (back)
   0.5f, 0.5f,  0.5f,  GREEN,    //Top Right (front)

  //Bottom
  //Triangle 1
   0.5f, -0.5f, -0.5f, RED,     // Bottom Right (back)
   0.5f, -0.5f,  0.5f, BLUE,    // Bottom Right (front)
  -0.5f, -0.5f, -0.5f, GREEN,   // Bottom Left (back)
  //Triangle 2
  -0.5f, -0.5f,  0.5f, RED,     //Bottom Left (front)
  -0.5f, -0.5f, -0.5f, BLUE,    //Bottom Left (back)
  0.5f,  -0.5f,  0.5f, GREEN,   //Bottom Right (front)
};

//Camera vectors
glm::vec3 eye  = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 gaze = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 up   = glm::vec3(0.0f, 0.1f, 0.0f);

bool controlIsPressed(GLFWwindow* window) {
	return glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}

//Prints the current coordinates of a given vector
void printCurrentCoordinates(glm::vec3 vec){
  std::cout << "(" << vec.x << ", "
                   << vec.y << ", "
                   << vec.z << ")" << std::endl;
}

// callback for keyboard input
// move camera when arrow keys are pressed, rotate it when arrow keys are pressed with control
void key_callback(GLFWwindow* mWindow, int key, int scancode, int action, int mods)
{
  GLfloat speed = 0.05f;
  //When you press shift, it will print out to the console informing you are pressing Shift otherwise it will be null
  std::cout << ((mods == GLFW_MOD_SHIFT) ? "You Pressed Shift!" : "") << std::endl;

  if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
    if (mods == GLFW_MOD_SHIFT){
      std::cout << "rotating camera left; new coordinates are: "; // rotate camera
      //In order to rotate the camera left you change x and z (positively)
      eye.x += speed;
      eye.z += speed;
      printCurrentCoordinates(eye);
    }
    else{
      std::cout << "moving camera left; new coordinates are: ";  // move camera
      gaze.x += speed;
      printCurrentCoordinates(gaze);
    }
  }
    // etc.
  else if(key == GLFW_KEY_RIGHT && action != GLFW_RELEASE){
    if(mods == GLFW_MOD_SHIFT){
      std::cout << "rotating camera right; new coordinates are: ";
      //In order to rotate the camera right, you change x and z (negatively)
      eye.x -= speed;
      eye.z -= speed;
      printCurrentCoordinates(eye);
    }
    else{
      std::cout << "moving camera right; new coordinates are: ";
      //To move the camera to the right, you change the gaze accordingly
      gaze.x -= speed;
      printCurrentCoordinates(gaze);
    }
  }

  else if(key == GLFW_KEY_UP && action != GLFW_RELEASE){
    if(mods == GLFW_MOD_SHIFT){
      std::cout << "rotating camera up; new coordinates are: ";
      //Now to rotate up, you change the y and z coordinates
      eye.y -= speed;
      eye.z -= speed;
      printCurrentCoordinates(eye);
    }
    else{
      std::cout << "moving camera up; new coordinates are: ";
      gaze.y -= speed;
      printCurrentCoordinates(gaze);
    }
  }

  else if(key == GLFW_KEY_DOWN && action != GLFW_RELEASE){
    if(mods == GLFW_MOD_SHIFT){
      std::cout << "rotating camera down; new coordinates are: ";
      //Now to rotate down, you change the y and z coordinates (negatively)
      eye.y -= speed;
      eye.z -= speed;
      printCurrentCoordinates(eye);
    }
    else{
      std::cout << "moving camera down; new coordinates are: ";
      gaze.y += speed;
      printCurrentCoordinates(gaze);
    }
  }
}




int main(int argc, char * argv[]) {
  //Clock start time to keep track of execution time
  clock_t cl1 = clock();

  // Load GLFW and Create a Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  auto mWindow = glfwCreateWindow(800, 600, "CISC3620", nullptr, nullptr);
  // Check for Valid Context
  if (mWindow == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    return EXIT_FAILURE;
  }

  // callbacks
  glfwSetKeyCallback(mWindow, key_callback);

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(mWindow);
  gladLoadGL();
  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  //glEnable(GL_DEPTH_TEST);
  // Create Vertex Array Object: this will store all the information about the vertex data that we are about to specify
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and compile the vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  // Create and compile the fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Specify the layout of the vertex data
  // position
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);  // attribute location, # values, value type, normalize?, stride, offset
  // color
  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  // Rendering Loop
  while (glfwWindowShouldClose(mWindow) == false) {
    //Clock 2 to keep to keep track of execution time
    clock_t cl2 = clock();
    double time_delta = (double)cl2 - (double)cl1;
    double seconds = time_delta/CLOCKS_PER_SEC;

    // Background Fill Color
    glClearColor(0.85f, 0.65f, 0.65f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT);

    //Model Transformation
    GLint modelTransform = glGetUniformLocation(shaderProgram, "model");
    //Created a rotation along the y axis starting from a 45 degree angle and rotating it by a factor of the execution time and taking the sin of it.
    glm::mat4 scale_model = glm::scale(glm::mat4(1.0), glm::vec3(0.5f, 0.5f, 0.5f));
    //glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f,0.5f,0.5f)), 45.0f * (GLfloat)sin(seconds/2), glm::vec3(0, 1, 0));
    glm::mat4 model = scale_model;
    glUniformMatrix4fv(modelTransform, 1, GL_FALSE, glm::value_ptr(model));

    //View Transformation
    GLint viewTransform = glGetUniformLocation(shaderProgram, "view");
    glm::mat4 view = glm::lookAt(eye, gaze, up);
    glUniformMatrix4fv(viewTransform, 1, GL_FALSE, glm::value_ptr(view));


    //Projection Transformation
    GLint projectTransform = glGetUniformLocation(shaderProgram, "projection");
    glm::mat4 project = glm::perspective(45.0f, WIDTH/HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projectTransform, 1, GL_FALSE, glm::value_ptr(project));

    // draw triangle
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0]));

    // Flip Buffers and Draw
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }   glfwTerminate();

  // clean up
  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  return EXIT_SUCCESS;
}
