

/*              PLEASE READ:
For this assignment I utilized the file from OpenGLSample to walk 
through the tutorial. After walking through the steps in tutorial for module 3.
I created the pyramid with that code :)
*/

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"



using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vao2;
        GLuint vao3;
        GLuint vao4;
        GLuint vao5;
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint vbos2[2];
        GLuint vbos3[2];
        GLuint vbos4[2];
        GLuint vbos5[2];
        GLuint nIndices;    // Number of indices of the mesh
        GLuint nIndices2;
        GLuint nIndices3;
        GLuint nIndices4;
        GLuint nIndices5;

    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture id
    GLuint gTextureId;
    GLuint gTextureId2;
    GLuint gTextureId3;
    GLuint gTextureId4;

    static glm::vec2 gUVScale(0.45f, 0.5f);
    static glm::vec2 gUVScale2(12.0f, .50f);
    static glm::vec2 gUVScale3(1.0f, 1.0f);
        GLint gTexWrapMode = GL_REPEAT;
    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;
    GLuint gCylProgramId;
    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(2.0f);

    // Cube and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
    glm::vec3 gLightScale(0.3f);
}
glm::mat4 viewMatrix;

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void UCreateMesh(GLMesh& mesh, int meshNum);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec2 textureCoordinate;  // Color data from Vertex Attrib Pointer 1

    out vec2 vertexTextureCoordinate; // variable to transfer color data to the fragment shader

    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    uniform sampler2D uTexture;
    uniform vec2 uvScale;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale); // Sends texture to the GPU for rendering
}
);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);




// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh, 1); // Calls the function to create the Vertex Buffer Object
    UCreateMesh(gMesh, 2);
    UCreateMesh(gMesh, 3);
    UCreateMesh(gMesh, 4);
    UCreateMesh(gMesh, 5);
    // Create the shader program
    /*if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
        */

    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCylProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;
    // Load texture (relative to project's directory)
    const char* texFilename = "../OpenGLSample/recourses/white.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    const char* texFilename1 = "../OpenGLSample/recourses/green2.jpg";
    if (!UCreateTexture(texFilename1, gTextureId2))
    {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }
    const char* texFilename2 = "../OpenGLSample/recourses/mouspad.jpg";
    if (!UCreateTexture(texFilename2, gTextureId3))
    {
        cout << "Failed to load texture " << texFilename2 << endl;
        return EXIT_FAILURE;
    }
    const char* texFilename3 = "../OpenGLSample/recourses/brick1.jpg";
    if (!UCreateTexture(texFilename3, gTextureId4))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCylProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCylProgramId, "uTexture"), 0);
    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //takes away the mouse issue i was having

    while (!glfwWindowShouldClose(gWindow))
    {
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);
    //releases texture data
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTextureId2);
    UDestroyTexture(gTextureId3);
    UDestroyTexture(gTextureId4);
    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gCylProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}
// Functioned called to render a frame
void URender()
{
   
    // Enable z-depth
    glEnable(GL_DEPTH_TEST); // keeps the point that is suppose to be infront drawn

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*WHITE CYLINDER START*/
    // 1. Scales the object by 2
    glUseProgram(gCylProgramId);

    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotates shape by 15 degrees in the y axis
    glm::mat4 rotation = glm::rotate(1.6f, glm::vec3(1.0f, 0.0f, 0.0f)); 
    // 3. placed at origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, -2.0f, -6.0f));

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;
    //glm::mat4 transformation(1.0f);
    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    

    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        GLint modelLoc = glGetUniformLocation(gCylProgramId, "model");
        GLint viewLoc = glGetUniformLocation(gCylProgramId, "view");
        GLint projLoc = glGetUniformLocation(gCylProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        GLint objectColorLoc = glGetUniformLocation(gCylProgramId, "objectColor");
        GLint lightColorLoc = glGetUniformLocation(gCylProgramId, "lightColor");
        GLint lightPositionLoc = glGetUniformLocation(gCylProgramId, "lightPos");
        GLint viewPositionLoc = glGetUniformLocation(gCylProgramId, "viewPosition");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
        glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
        glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
        const glm::vec3 cameraPosition = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        GLint UVScaleLoc = glGetUniformLocation(gCylProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(gMesh.vao);
        // bind textures on corresponding texture units

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        // Draws the triangles
        glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
        
        // Deactivate the Vertex Array Object
        glBindVertexArray(0);

        /*WHITE CYLINDER END*/
        /*GREEN CYLINDER BEGIN*/
        glUseProgram(gCylProgramId);
        scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
        // 2. Rotates shape by 15 degrees in the y axis
        rotation = glm::rotate(1.6f, glm::vec3(1.0f, .0f, 0.0f));
        // 3. placed at origin
        translation = glm::translate(glm::vec3(0.0f, 0.0f, -6.0f));

        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation * scale;
        //glm::mat4 transformation(1.0f);
        // camera/view transformation
        view = gCamera.GetViewMatrix();



        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        modelLoc = glGetUniformLocation(gCylProgramId, "model");
        viewLoc = glGetUniformLocation(gCylProgramId, "view");
        projLoc = glGetUniformLocation(gCylProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        objectColorLoc = glGetUniformLocation(gCylProgramId, "objectColor");
        lightColorLoc = glGetUniformLocation(gCylProgramId, "lightColor");
        lightPositionLoc = glGetUniformLocation(gCylProgramId, "lightPos");
        viewPositionLoc = glGetUniformLocation(gCylProgramId, "viewPosition");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
        glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
        glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
        const glm::vec3 cameraPosition1 = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition1.x, cameraPosition1.y, cameraPosition1.z);

        UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale2));

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(gMesh.vao2);
        // bind textures on corresponding texture units

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextureId2);
        // Draws the triangles
        glDrawElements(GL_TRIANGLES, gMesh.nIndices2, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

        // Deactivate the Vertex Array Object
        glBindVertexArray(0);

        /* END OF GREEN CYLINDER*/

        /* BEGIN OF PLANE*/
               

        scale = glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
        // 2. Rotates shape by 15 degrees in the y axis
        rotation = glm::rotate(15.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. placed at origin
        translation = glm::translate(glm::vec3(0.0f, -4.0f, 0.0f));

        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation * scale;
        //glm::mat4 transformation(1.0f);
        // camera/view transformation
        view = gCamera.GetViewMatrix();



        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        modelLoc = glGetUniformLocation(gCylProgramId, "model");
        viewLoc = glGetUniformLocation(gCylProgramId, "view");
        projLoc = glGetUniformLocation(gCylProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        objectColorLoc = glGetUniformLocation(gCylProgramId, "objectColor");
        lightColorLoc = glGetUniformLocation(gCylProgramId, "lightColor");
        lightPositionLoc = glGetUniformLocation(gCylProgramId, "lightPos");
        viewPositionLoc = glGetUniformLocation(gCylProgramId, "viewPosition");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
        glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
        glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
        const glm::vec3 cameraPosition2 = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition2.x, cameraPosition2.y, cameraPosition2.z);

        UVScaleLoc = glGetUniformLocation(gCylProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale3));

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(gMesh.vao3);
        // bind textures on corresponding texture units

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextureId3);
        // Draws the triangles
        glDrawElements(GL_TRIANGLES, gMesh.nIndices3, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

        // Deactivate the Vertex Array Object
        glBindVertexArray(0);


        /*PYRAMID START*/

        scale = glm::scale(glm::vec3(4.0f, 2.0f, 4.0f));
        // 2. Rotates shape by 15 degrees in the y axis
        rotation = glm::rotate(4.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. placed at origin
        translation = glm::translate(glm::vec3(4.0f, -3.0f, 6.0f));

        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation * scale;
        //glm::mat4 transformation(1.0f);
        // camera/view transformation
        view = gCamera.GetViewMatrix();



        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        modelLoc = glGetUniformLocation(gCylProgramId, "model");
        viewLoc = glGetUniformLocation(gCylProgramId, "view");
        projLoc = glGetUniformLocation(gCylProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        objectColorLoc = glGetUniformLocation(gCylProgramId, "objectColor");
        lightColorLoc = glGetUniformLocation(gCylProgramId, "lightColor");
        lightPositionLoc = glGetUniformLocation(gCylProgramId, "lightPos");
        viewPositionLoc = glGetUniformLocation(gCylProgramId, "viewPosition");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
        glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
        glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
        const glm::vec3 cameraPosition3 = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition3.x, cameraPosition3.y, cameraPosition3.z);

        UVScaleLoc = glGetUniformLocation(gCylProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale3));

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(gMesh.vao4);
        // bind textures on corresponding texture units

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextureId4);
        // Draws the triangles
        glDrawElements(GL_TRIANGLES, gMesh.nIndices4, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

        // Deactivate the Vertex Array Object
        glBindVertexArray(0);



        /*BEGIN CUBE*/
        scale = glm::scale(glm::vec3(4.0f, 3.0f, 2.0f));
        // 2. Rotates shape by 15 degrees in the y axis
        rotation = glm::rotate(2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. placed at origin
        translation = glm::translate(glm::vec3(-5.0f, -3.0f, 4.0f));

        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation * scale;
        //glm::mat4 transformation(1.0f);
        // camera/view transformation
        view = gCamera.GetViewMatrix();



        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        modelLoc = glGetUniformLocation(gCylProgramId, "model");
        viewLoc = glGetUniformLocation(gCylProgramId, "view");
        projLoc = glGetUniformLocation(gCylProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        objectColorLoc = glGetUniformLocation(gCylProgramId, "objectColor");
        lightColorLoc = glGetUniformLocation(gCylProgramId, "lightColor");
        lightPositionLoc = glGetUniformLocation(gCylProgramId, "lightPos");
        viewPositionLoc = glGetUniformLocation(gCylProgramId, "viewPosition");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
        glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
        glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
        const glm::vec3 cameraPosition4 = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition4.x, cameraPosition4.y, cameraPosition4.z);

        UVScaleLoc = glGetUniformLocation(gCylProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale3));

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(gMesh.vao5);
        // bind textures on corresponding texture units

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTextureId4);
        // Draws the triangles
        glDrawElements(GL_TRIANGLES, gMesh.nIndices5, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

        // Deactivate the Vertex Array Object
        glBindVertexArray(0);
        
    // Set the shader to be used

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh, int num)
{    // Vertex data
    /*
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)     //texture
        //top of pyramid
        0.0f, 0.5f, 0.0f,          0.5f, 1.0f,     // 0 top
        //back base
        -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,     // 1 bottom left
        0.5f, -0.5f, -0.5f,      1.0f, 0.0f,     // 2 bottom right

        0.0f, 0.5f, 0.0f,          0.5f, 1.0f,
        -0.5f, -0.5f, 0.5f,          0.0f,0.0f,
        0.5f, -0.5f, 0.5f,          1.0f,0.0f,

        0.0f, 0.5f, 0.0f,           0.5f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,

        0.0f, 0.5f, 0.0f,       0.5, 1.0f,
        0.5f, -0.5f, 0.5f,     0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,    1.0f, 0.0f,

        0.5f, -0.5, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f,0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 1.0f,

        -0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f,


        // front base
        -0.5f, -0.5f, 0.5f,       1.0f, 0.0f,      // 3 front left bottom
        0.5f, -0.5f, 0.5f,     1.0f, 1.0f,        // 4 front right bottom

        0.0f, 0.5f, 0.0f,      0.0f, .0f,     // 5 top
        //-0.5
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,     // 6 bottom left
        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f // 7 front left


    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2, //triangle one
        3, 4, 5, // triangle 2
        6, 7, 8, // triangle 3
        9, 10, 11, //triangle 4
        12, 13, 14, // triangle 5
        15, 16, 17 //triangle 6

    };*/


    std::vector <GLfloat> verts = {
    };
    std::vector<GLfloat> combined = {};
    // Index data to share position data
    std::vector <GLushort> indices = {
    };
    /*WHITE CYLINDER*/
    std::vector <GLfloat>().swap(verts);
    std::vector <GLushort>().swap(indices);
    std::vector <GLfloat>().swap(combined);
    if (num == 1) {
        const float PI = 3.1415926f;
        int sectorCount = 20;
        float sectorStep = 2 * PI / sectorCount;
        float sectorAngle;  // radian
        int baseCenterIndex = (int)verts.size() / 3;
        std::vector<float> unitCircleVertices;
        for (int i = 0; i <= sectorCount; ++i)
        {
            sectorAngle = i * sectorStep;
            unitCircleVertices.push_back(cos(sectorAngle)); // x
            unitCircleVertices.push_back(sin(sectorAngle)); // y
            unitCircleVertices.push_back(0);                // z


        }


        std::vector<float> unitVertices = unitCircleVertices;
        std::vector<float>().swap(verts);
        float height = 2.0f;
        for (int i = 0; i < 2; ++i)
        {
            float h = (-height / 2.0f) + (i * height);           // z value; -h/2 to h/2
            float radius = .5f;
            float t = 1.0f - i; //t
            for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
            {
                float ux = unitVertices[k];
                float uy = unitVertices[k + 1];
                float uz = unitVertices[k + 2];
                // position vector
                verts.push_back(ux * radius);             // vx
                combined.push_back(ux * radius);
                verts.push_back(uy * radius);             // vy
                combined.push_back(uy * radius);
                verts.push_back(h);                       // vz
                combined.push_back(h);
                combined.push_back(ux);
                combined.push_back(uy);
                combined.push_back(uz);
                combined.push_back((float)j / sectorCount);

                combined.push_back(t);
            }
        }
        baseCenterIndex = (int)verts.size() / 3;
        //int topCenterIndex = baseCenterIndex + sectorCount + 1;
        for (int i = 0; i < 2; ++i)
        {
            float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
            float nz = -1 * i * 2;
            verts.push_back(0.0f);
            combined.push_back(0.0f);
            verts.push_back(0.0f);
            combined.push_back(0.0f);
            verts.push_back(h);
            combined.push_back(h);
            //verts.push_back(0.5f);
            combined.push_back(0.0f);
            combined.push_back(0.0f);
            combined.push_back(nz);
            combined.push_back(0.5f);
            //verts.push_back(0.5f);
            combined.push_back(0.5f);
            float radius = .5f;
            int k = 0;
            for (int j = 0; j <= sectorCount; ++j)
            {

                float ux = unitVertices[k];
                float uy = unitVertices[k + 1];
                // position vector
                verts.push_back(ux * radius);             // vx
                combined.push_back(ux * radius);
                verts.push_back(uy * radius);             // vy
                combined.push_back(uy * radius);
                combined.push_back(h);
                verts.push_back(h);
                combined.push_back(0.0f);
                combined.push_back(0.0f);
                combined.push_back(nz);
                combined.push_back(-ux * 0.5f + 0.5f);      // s
                combined.push_back(-uy * 0.5f + 0.5f);      // t
                k += 3;


            }
        }

        int topCenterIndex = (unsigned int)indices.size() / 3;

        int k1 = 0;                         // 1st vertex index at base
        int k2 = sectorCount + 1;           // 1st vertex index at top

        // indices for the side surface

        for (int i = 0; i < sectorCount; ++i, ++k1, ++k2)
        {
            // 2 triangles per sector
            // k1 => k1+1 => k2
            indices.push_back(k1);
            indices.push_back(k1 + 1);
            indices.push_back(k2);

            // k2 => k1+1 => k2+1
            indices.push_back(k2);
            indices.push_back(k1 + 1);
            indices.push_back(k2 + 1);
        }

        // indices for the base surface
        //NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation


        /*for (int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
            {
                indices.push_back(baseCenterIndex);
                indices.push_back(k + 1);
                indices.push_back(k);
            }
            else // last triangle
            {
                indices.push_back(baseCenterIndex);
                indices.push_back(baseCenterIndex + 1);
                indices.push_back(k);
            }
        }*/
        //topCenterIndex = (unsigned int)indices.size();
        // indices for the top surface
        for (int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
            {
                indices.push_back(topCenterIndex);
                indices.push_back(k);
                indices.push_back(k + 1);
            }
            else // last triangle
            {
                indices.push_back(topCenterIndex);
                indices.push_back(k);
                indices.push_back(topCenterIndex + 1);
            }
        }




        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;


        glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, combined.size() * floatsPerVertex, &combined[0], GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices = combined.size() * (1 * (floatsPerVertex + floatsPerNormal + floatsPerUV));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * floatsPerVertex, &indices[0], GL_STATIC_DRAW);

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }

    if (num == 2) {

        const float PI = 3.1415926f;
        int sectorCount = 20;
        float sectorStep = 2 * PI / sectorCount;
        float sectorAngle;  // radian
        int baseCenterIndex = (int)verts.size() / 3;
        std::vector<float> unitCircleVertices;
        for (int i = 0; i <= sectorCount; ++i)
        {
            sectorAngle = i * sectorStep;
            unitCircleVertices.push_back(cos(sectorAngle)); // x
            unitCircleVertices.push_back(sin(sectorAngle)); // y
            unitCircleVertices.push_back(0);                // z


        }


        std::vector<float> unitVertices = unitCircleVertices;
        std::vector<float>().swap(verts);
        float height = 1.0f;
        for (int i = 0; i < 2; ++i)
        {
            float h = (-height / 2.0f) + (i * height);           // z value; -h/2 to h/2
            float radius = .45f;
            float t = 1.0f - i; //t
            for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
            {
                float ux = unitVertices[k];
                float uy = unitVertices[k + 1];
                float uz = unitVertices[k + 2];
                // position vector
                verts.push_back(ux * radius);             // vx
                combined.push_back(ux * radius);
                verts.push_back(uy * radius);             // vy
                combined.push_back(uy * radius);
                verts.push_back(h);                       // vz
                combined.push_back(h);
                combined.push_back(ux);
                combined.push_back(uy);
                combined.push_back(uz);
                combined.push_back((float)j / sectorCount);

                combined.push_back(t);
            }
        }
        baseCenterIndex = (int)verts.size() / 3;
        //int topCenterIndex = baseCenterIndex + sectorCount + 1;
        for (int i = 0; i < 2; ++i)
        {
            float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
            float nz = -1 * i * 2;
            verts.push_back(0.0f);
            combined.push_back(0.0f);
            verts.push_back(0.0f);
            combined.push_back(0.0f);
            verts.push_back(h);
            combined.push_back(h);
            //verts.push_back(0.5f);
            combined.push_back(0.0f);
            combined.push_back(0.0f);
            combined.push_back(nz);
            combined.push_back(0.5f);
            //verts.push_back(0.5f);
            combined.push_back(0.5f);
            float radius = .5f;
            int k = 0;
            for (int j = 0; j <= sectorCount; ++j)
            {

                float ux = unitVertices[k];
                float uy = unitVertices[k + 1];
                // position vector
                verts.push_back(ux * radius);             // vx
                combined.push_back(ux * radius);
                verts.push_back(uy * radius);             // vy
                combined.push_back(uy * radius);
                combined.push_back(h);
                verts.push_back(h);
                combined.push_back(0.0f);
                combined.push_back(0.0f);
                combined.push_back(nz);
                combined.push_back(-ux * 0.5f + 0.5f);      // s
                combined.push_back(-uy * 0.5f + 0.5f);      // t
                k += 3;


            }
        }

        int topCenterIndex = (unsigned int)indices.size() / 3;

        int k1 = 0;                         // 1st vertex index at base
        int k2 = sectorCount + 1;           // 1st vertex index at top

        // indices for the side surface

        for (int i = 0; i < sectorCount; ++i, ++k1, ++k2)
        {
            // 2 triangles per sector
            // k1 => k1+1 => k2
            indices.push_back(k1);
            indices.push_back(k1 + 1);
            indices.push_back(k2);

            // k2 => k1+1 => k2+1
            indices.push_back(k2);
            indices.push_back(k1 + 1);
            indices.push_back(k2 + 1);
        }

        // indices for the base surface
        //NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation


        /*for (int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
            {
                indices.push_back(baseCenterIndex);
                indices.push_back(k + 1);
                indices.push_back(k);
            }
            else // last triangle
            {
                indices.push_back(baseCenterIndex);
                indices.push_back(baseCenterIndex + 1);
                indices.push_back(k);
            }
        }*/
        //topCenterIndex = (unsigned int)indices.size();
        // indices for the top surface
        for (int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
            {
                indices.push_back(topCenterIndex);
                indices.push_back(k);
                indices.push_back(k + 1);
            }
            else // last triangle
            {
                indices.push_back(topCenterIndex);
                indices.push_back(k);
                indices.push_back(topCenterIndex + 1);
            }
        }




        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;


        glGenVertexArrays(1, &mesh.vao2); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao2);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos2[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, combined.size() * floatsPerVertex, &combined[0], GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices2 = combined.size() * (1 * (floatsPerVertex + floatsPerNormal + floatsPerUV));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos2[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * floatsPerVertex, &indices[0], GL_STATIC_DRAW);

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }

    /*END OF GREEN CYLINDER*/

    /*START OF PLANE*/

    if (num == 3) {
        GLfloat verts1[] = {
            // vertices             // tex coords
                -1.0f, 0.0f, 1.0f,   1.0f, 0.0f, -1.0f   ,0.0f, 0.0f,
                -1.0f, 0.0f, -1.0f,   -1.0f, 1.0f, 0.0f  ,1.0f, 0.0f,
                1.0f, 0.0f, 1.0f,      1.0f, 0.0f, -1.0f ,0.0f, 1.0f,
                1.0f, 0.0f, -1.0f,     -1.0f, 1.0f, 0.0f ,1.0f, 1.0f

        };
        GLushort indices1[] = {
            0, 1, 2, //triangle one
            1, 2,3 // triangle 2
        };
        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;


        glGenVertexArrays(1, &mesh.vao3); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao3);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos3);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos3[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts1), verts1, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices3 = sizeof(indices) / sizeof(indices[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos3[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);

    }
    /*END OF PLANE*/

    /*START OF PYRAMID*/
    
    if (num == 4) {
        GLfloat verts1[] = {
            // vertices             // tex coords
// Vertex Positions    // Colors (r,g,b,a)     //texture
        //top of pyramid
        0.0f, 0.5f, 0.0f,      1.0f, -1.0f, 0.0f,      0.5f, 1.0f,     // 0 top
        //back base
        -0.5f, -0.5f, -0.5f,   1.0f, -1.0f, 0.0f,      0.0f, 0.0f,     // 1 bottom left
        0.5f, -0.5f, -0.5f,    1.0f, -1.0f, 0.0f,      1.0f, 0.0f,     // 2 bottom right

        0.0f, 0.5f, 0.0f,      -1.0f, 1.0f, 0.0f,      0.5f, 1.0f,
        -0.5f, -0.5f, 0.5f,    -1.0f, 1.0f, 0.0f,      0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,     -1.0f, 1.0f, 0.0f,      1.0f, 0.0f,

        0.0f, 0.5f, 0.0f,      1.0f, 1.0f, 0.0f,      0.5f, 1.0f,
        -0.5f, -0.5f, 0.5f,    1.0f, 1.0f, 0.0f,      0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 0.0f,      1.0f, 0.0f, 

        0.0f, 0.5f, 0.0f,      -1.0f, -1.0f, 0.0f,      0.5f, 1.0f,
        0.5f, -0.5f, 0.5f,     -1.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,    -1.0f, -1.0f, 0.0f,      1.0f, 0.0f,

        0.5f, -0.5, -0.5f,     0.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f,      1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 0.0f,      1.0f, 1.0f,

        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,      0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,      0.0f, 0.0f,


        // front base
        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 0.0f,      1.0f, 0.0f,      // 3 front left bottom
        0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,      1.0f, 1.0f,        // 4 front right bottom

        0.0f, 0.5f, 0.0f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f,     // 5 top
        //-0.5
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f,      0.0f, 0.0f,     // 6 bottom left
        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 0.0f,      0.0f, 0.0f // 7 front left


        };
        GLushort indices1[] = {
        0, 1, 2, //triangle one
        3, 4, 5, // triangle 2
        6, 7, 8, // triangle 3
        9, 10, 11, //triangle 4
        12, 13, 14, // triangle 5
        15, 16, 17 //triangle 6
        };
        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;


        glGenVertexArrays(1, &mesh.vao4); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao4);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos4);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos4[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts1), verts1, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices4 = sizeof(indices1) / sizeof(indices1[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos4[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }
    /*END OF BOX*/
    /*START OF PYRAMID*/
    if (num == 5) {
        
        GLfloat verts1[] = { 
         -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
            //6
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
        //12
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,     0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
        //18
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 0.0f,     1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 0.0f,     1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 0.0f,     0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         //24
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,     1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 1.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        //30
        -0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 1.0f,     0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 1.0f,     0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 1.0f,     1.0f, 0.0f
        };
        GLushort indices1[] = {
            0,1,2,
            3,4,5,
            6,7,8,
            9,10,11,
            12,13,14,
            15,16,17,
            18,19,20,
            21,22,23,
            24,25,26,
            27,28,29,
            30,31,32,
            33,34,35
        };


        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;


        glGenVertexArrays(1, &mesh.vao5); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao5);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos5);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos5[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts1), verts1, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices5 = sizeof(indices1) / sizeof(indices1[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos5[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }
    /*END OF PYRAMID*/
}



/*Generate and load the texture*/
bool UCreateTexture(const char* filename ,GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
      
    
            flipImageVertically(image, width, height, channels);
       

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    
    glDeleteVertexArrays(1, &mesh.vao2);
    glDeleteVertexArrays(1, &mesh.vao3);
    glDeleteVertexArrays(1, &mesh.vao4);
    glDeleteVertexArrays(1, &mesh.vao5);
    glDeleteBuffers(2, mesh.vbos);
    glDeleteBuffers(2, mesh.vbos2);
    glDeleteBuffers(2, mesh.vbos3);
    glDeleteBuffers(2, mesh.vbos4);
    glDeleteBuffers(2, mesh.vbos5);

}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
