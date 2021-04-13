#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../utils/camera.h"
#include "../utils/shader.h"
#include "../utils/draw_shapes.h"
#include "physics.h"

#include <Eigen/Dense>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0f;    // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Mouse offset;
float mouseOffsetX, mouseOffsetY;
const float dragSensitivity = 0.01f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

Camera camera(Eigen::Vector3f(0.0f, 0.0f, 3.0f));

int main(int argc, const char * argv[]) {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mass spring simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // Setting up shader.
    Shader lightingShader("shaders/vertex.vs", "shaders/fragment.fs");
    lightingShader.use();

    Eigen::Matrix4f projection;
    projection = camera.GetPerspectiveMatrix(800.0f / 600.0f, 0.1f, 100.0f);
    lightingShader.setMat4("projection", projection);

    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    lightingShader.setMat4("model", model);
    
    // Setting up mesh and physical system.
    Mesh mesh = sphereMesh(8);
    long n = mesh.positions.size();
    // Making stiffness proportional to number of vertices.
    float k = n * 1.0f;
    float m = 1.0f;
    PhysicalMesh pm = PhysicalMesh(mesh,m,k,10.0f);
    
    float h = 0.01;
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        
        // input
        processInput(window);
        
        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Eigen::Matrix4f view = camera.GetViewMatrix();
        lightingShader.setMat4("view", view);

        pm.moveFixedPoints(
           Eigen::Vector3f(-mouseOffsetY*dragSensitivity,
                           -mouseOffsetX*dragSensitivity,
                           0));
        
        pm.simulationStep(h);
        
        //Updating original mesh with new positions.
        pm.updateMesh(mesh);
        //Rendering original mesh.
        renderMesh(mesh, sphereVAO);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    Camera_Movement direction = Camera_Movement::NONE;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        direction = Camera_Movement::FORWARD;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        direction = Camera_Movement::BACKWARD;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        direction = Camera_Movement::LEFT;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        direction = Camera_Movement::RIGHT;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        direction = Camera_Movement::UP;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        direction = Camera_Movement::DOWN;
    
    camera.ProcessKeyboard(direction, deltaTime);
}

float lastX = 400, lastY = 300;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    mouseOffsetX = xpos - lastX;
    mouseOffsetY = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    mouseOffsetX *= sensitivity;
    mouseOffsetY *= sensitivity;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
