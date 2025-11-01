#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "game.h"

glm::vec2 desiredDir; // (-1,0) left; (1,0) right; (0,1) up; (0,-1) down

float window_width = 800.0f;
float window_height = 600.0f;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

int main()
{
  // Initialize GLFW
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Configure GLFW for OpenGL 4.6 Core Profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window
  GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Pacman Clone", NULL, NULL);
  if (!window)
  {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Load GLAD (must be after making context current)
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "Failed to initialize GLAD\n");
    return -1;
  }

  // Print OpenGL info
  printf("🎮 OpenGL Version: %s\n", glGetString(GL_VERSION));
  printf("🎨 GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  printf("🖥️  Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Press ESC to exit\n\n");

  // Set viewport
  glViewport(0, 0, window_width, window_height);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Game game;

  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glm::mat4 projection = glm::ortho(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);
    game.Draw(projection);

    glfwSwapBuffers(window);
    glfwPollEvents();
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    game.pacman.updateTexture(deltaTime);

    process_input(window);
    game.PhysicsUpdate(deltaTime, desiredDir);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  printf("🔄 Window resized to %dx%d\n", width, height);
  window_width = width;
  window_height = height;
  glViewport(0, 0, width, height);
}

// Process input
void process_input(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, 1);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    desiredDir = glm::vec2(0, -1);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    desiredDir = glm::vec2(0, 1);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    desiredDir = glm::vec2(-1, 0);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    desiredDir = glm::vec2(1, 0);
}
