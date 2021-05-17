#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../utils/camera.h"
#include "../utils/shader.h"
#include "../utils/draw_shapes.h"
#include "../utils/RootDir.h"
#include "./physics.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <regex>

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
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

Camera camera(Eigen::Vector3f(0.0f, 0.0f, 9.0f));

int main(int argc, const char * argv[]) {
    string path_prefix = string(ROOT_DIR) + "src/3d_fem/";

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FEM 3d simulation", NULL, NULL);
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // Setting up shader.
    
    Shader trivialShader(path_prefix+"shaders/trivial.vs", path_prefix+"shaders/trivial.fs");
    Shader pbrShader(path_prefix+"shaders/vertex.vs", path_prefix+"shaders/fragment.fs");
    
    pbrShader.use();
    Eigen::Matrix4f projection;
    projection = camera.GetPerspectiveMatrix(800.0f / 600.0f, 0.1f, 100.0f);
    pbrShader.setMat4("projection", projection);
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    pbrShader.setMat4("model", model);
    Eigen::Vector3f pointLightPositions[] = {
        Eigen::Vector3f( 1.0f,  1.0f,  2.0f),
        Eigen::Vector3f( 2.3f, -3.3f, -4.0f),
        Eigen::Vector3f(-4.0f,  2.0f, -12.0f),
        Eigen::Vector3f( 0.0f,  0.0f, -3.0f)
    };
    for(int i = 0; i< 4; i++){
        pbrShader.setVec3("pointLights["+to_string(i)+"].position", pointLightPositions[i]);
    }
    Eigen::Vector3f bunnyAlbedo(1.0, 0.4, 0.7);
    pbrShader.setVec3("albedo", bunnyAlbedo);
    pbrShader.setFloat("roughness", 0.01f);
    pbrShader.setFloat("metallic", 0.01f);
    
    Eigen::Vector3f cubeAlbedo(0.9, 0.9, 0.9);
    trivialShader.use();
    trivialShader.setMat4("projection", projection);
    trivialShader.setMat4("model", model);
    trivialShader.setVec3("albedo", cubeAlbedo);

    Mesh cubeMesh(path_prefix + "mesh/big_cube.obj");
    Eigen::Vector3f cubeDisplacement(0,-5.5,0);
    for(auto &v: cubeMesh.positions){
        v += cubeDisplacement;
    }
    
    TetrahedralMesh tetMesh(path_prefix + "mesh/bunny_tet.msh");
    Mesh skinMesh(path_prefix + "mesh/bunny.obj");
    PhysicalMesh pm = PhysicalMesh(tetMesh, skinMesh);
    
    unsigned int bunnyVAO, bunnyVBO = 0;
    unsigned int cubeVAO, cubeVBO = 0;
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
        
        pm.simulationStep();
        Mesh updatedMesh = pm.getSkinMesh();
        
        pbrShader.use();
        Eigen::Matrix4f view = camera.GetViewMatrix();
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);
        renderMesh(updatedMesh, bunnyVAO, bunnyVBO);
        
        trivialShader.use();
        trivialShader.setMat4("view", view);
        renderMesh(cubeMesh, cubeVAO, cubeVBO);
        
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
