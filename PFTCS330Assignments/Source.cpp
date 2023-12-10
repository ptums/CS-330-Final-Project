#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <numbers>

#include <learnOpengl/camera.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "7.1 Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // object meshes
    GLMesh gKeyboardMesh, gGolfBallMesh, gAirPodMesh, gGlassMesh, gDeskMesh, gDeskMatMesh;

    // Shader program
    GLuint gProgramId;

    // Texture id
    GLuint gTextureKeyboardId, gTextureGolfBallId, gTextureAirPodId, gTextureGlassId, gTextureDeskId, gTextureDeskMatId;

    // Mesh variables
    GLuint floatsPerVertex, floatsPerUV, floatsPerNormal;

    // variables for drawing
    glm::vec2 gUVScale;
    glm::mat4 scale, rotation, translation, model, view, projection;
    GLint modelLoc, viewLoc, projLoc, uvScaleLoc;

    // Lighting

    // Light 1
    glm::vec3 gLightColorOne(0.4f, 0.4f, 0.4f); 
    glm::vec3 gLightPositionOne(2.0f, 4.0f, 2.0f);
    glm::vec3 gLightScaleOne(1.0f);


    // Light 2
    glm::vec3 gLightColorTwo(0.4f, 0.4f, 0.4f);
    glm::vec3 gLightPositionTwo(2.0f, 4.0f, 2.0f);
    glm::vec3 gLightScaleTwo(1.0f);


    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 5.5f)); // we create the camera view - x and y are at the center and z is 3 units away from the center

    float gLastX = WINDOW_WIDTH / 2.0f; // sets initial values for mouse movement tracking - what do on the x/y axis
    float gLastY = WINDOW_HEIGHT / 2.0f; // setting it a 2.0f sets in the center of the screen
    bool gFirstMouse = true;

    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    bool switchOrthoProj = false;

    //  Sets the scale for the texture coordinates for both the horizontal and vertical directions.
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateDeskMesh(GLMesh& mesh);
void UCreateKeyboardMesh(GLMesh& mesh);
void UCreateGolfBallMesh(GLMesh& mesh);
void UCreateAirPodMesh(GLMesh& mesh);
void UCreateGlassMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void UDestroyProgram();
bool UCreateTexture(const char* filename, GLuint& textureId, bool flipImg);
void UDestroyTexture(GLuint textureId);
void URender(GLFWwindow* window);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec2 textureCoordinate; // Text coords data from Vertex Attrib Pointer 2
    layout(location = 2) in vec3 normal;

    out vec3 vertexFragmentPos;
    out vec3 vertexNormal;
    out vec2 vertexTextureCoordinate;

    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f);
        vertexFragmentPos = vec3(model * vec4(position, 1.0f));
        vertexNormal = mat3(transpose(inverse(model))) * normal;
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexFragmentPos;
    in vec3 vertexNormal;
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    uniform vec3 lightColorOne;
    uniform vec3 lightPosOne;
    uniform vec3 viewPositionOne;
    uniform vec3 lightColorTwo;
    uniform vec3 lightPosTwo;
    uniform vec3 viewPositionTwo;

    uniform sampler2D uTexture;
    uniform vec2 uvScale;


    void main()
    {
        // Light One

        //Calculate Ambient lighting
        float ambientStrengthOne = 0.6f;
        vec3 ambientOne = ambientStrengthOne * lightColorOne;

        //Calculate Diffuse lighting
        vec3 normOne = normalize(vertexNormal);
        vec3 lightDirectionOne = normalize(lightPosOne - vertexFragmentPos);
        float impactOne = max(dot(normOne, lightDirectionOne), 0.6);
        vec3 diffuseOne = impactOne * lightColorOne;

        //Calculate Specular lighting
        float specularIntensityOne = 0.8f;
        float highlightSizeOne = 16.0f;
        vec3 viewDirOne = normalize(viewPositionOne - vertexFragmentPos);
        vec3 reflectDirOne = reflect(-lightDirectionOne, normOne);

        //Calculate specular component
        float specularComponentOne = pow(max(dot(viewDirOne, reflectDirOne), 0.1), highlightSizeOne);
        vec3 specularOne = specularIntensityOne * specularComponentOne * lightColorOne;


        // Light Two

        //Calculate Ambient lighting
        float ambientStrengthTwo = 0.6f;
        vec3 ambientTwo = ambientStrengthTwo * lightColorTwo;

        //Calculate Diffuse lighting
        vec3 normTwo = normalize(vertexNormal);
        vec3 lightDirectionTwo = normalize(lightPosTwo - vertexFragmentPos);
        float impactTwo = max(dot(normTwo, lightDirectionTwo), 0.6);
        vec3 diffuseTwo = impactTwo * lightColorTwo;

        //Calculate Specular lighting
        float specularIntensityTwo = 0.8f;
        float highlightSizeTwo = 16.0f;
        vec3 viewDirTwo = normalize(viewPositionTwo - vertexFragmentPos);
        vec3 reflectDirTwo = reflect(-lightDirectionTwo, normTwo);

        //Calculate specular component
        float specularComponentTwo = pow(max(dot(viewDirTwo, reflectDirTwo), 0.1), highlightSizeTwo);
        vec3 specularTwo = specularIntensityTwo * specularComponentTwo * lightColorTwo;

        // texture
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate phong result
        vec3 phong = (ambientOne + ambientTwo + diffuseOne + diffuseTwo + specularOne + specularTwo) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0);
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


/* Set up texture for creation */
bool setupTexture(const char* filename, GLuint& textureId, GLuint programId, const char* uniformName, bool flipImage)
{
    if (!UCreateTexture(filename, textureId, flipImage))
    {
        cout << "Failed to load texture " << filename << endl;
        return false;
    }

    glUseProgram(programId);
    glUniform1i(glGetUniformLocation(programId, uniformName), 0);

    return true;
}


/** set up object for transformation **/
glm::mat4 setupTransform(glm::mat4 scaleVec, glm::mat4 rotateVec, glm::mat4 translateVec)
{
    scale = scaleVec;
    rotation = rotateVec;
    translation = translateVec;

    return translation * rotation * scale;
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create mesh data
    UCreateDeskMesh(gDeskMesh);
    UCreateDeskMesh(gDeskMatMesh);
    UCreateKeyboardMesh(gKeyboardMesh);
    UCreateGolfBallMesh(gGolfBallMesh);
    UCreateAirPodMesh(gAirPodMesh);
    UCreateGlassMesh(gGlassMesh);

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Setup textures
    if (!setupTexture("../Resources/keyboard.png", gTextureKeyboardId, gProgramId, "gKeyboardTexture", false))
        return EXIT_FAILURE;

    if (!setupTexture("../Resources/golfball.png", gTextureGolfBallId, gProgramId, "golfBallTexture", true))
        return EXIT_FAILURE;

    if (!setupTexture("../Resources/airpods-4.png", gTextureAirPodId, gProgramId, "airPodTexture", true))
        return EXIT_FAILURE;

    if (!setupTexture("../Resources/cup-3.png", gTextureGlassId, gProgramId, "glassTexture", false))
        return EXIT_FAILURE;

    if (!setupTexture("../Resources/desk.png", gTextureDeskId, gProgramId, "deskTexture", false))
        return EXIT_FAILURE;

    if (!setupTexture("../Resources/desk-mat.png", gTextureDeskMatId, gProgramId, "deskMatTexture", false))
        return EXIT_FAILURE;


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame; // calculates the time elapsed between the current frame and the last frame in a real-time application
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender(gWindow);

        glfwPollEvents();
    }

    UDestroyProgram();
    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (switchOrthoProj == false)
        {
            switchOrthoProj = true;
        }
        else {
            switchOrthoProj = false;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Functioned called to render the keyboar object frames
void URender(GLFWwindow* window)
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // do an initial poaint and set everyting 

    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");


    glm::mat4 cameraView = gCamera.GetViewMatrix();

    if (switchOrthoProj == false) {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        projection = glm::ortho(0.0f, 200.0f, 0.0f, 200.0f, 0.1f, 100.0f);
    }


    /**
   * Lighting
   **/

   // Light One

    GLint lightColorLocOne = glGetUniformLocation(gProgramId, "lightColorOne");
    GLint lightPositionLocOne = glGetUniformLocation(gProgramId, "lightPosOne");
    GLint viewPositionLocOne = glGetUniformLocation(gProgramId, "viewPositionOne");

    glUniform3f(lightColorLocOne, gLightColorOne.r, gLightColorOne.g, gLightColorOne.b);
    glUniform3f(lightPositionLocOne, gLightPositionOne.x, gLightPositionOne.y, gLightPositionOne.z);
    const glm::vec3 cameraPositionOne = gCamera.Position;
    glUniform3f(viewPositionLocOne, cameraPositionOne.x, cameraPositionOne.y, cameraPositionOne.z);


    // Light Two

    GLint lightColorLocTwo = glGetUniformLocation(gProgramId, "lightColorTwo");
    GLint lightPositionLocTwo = glGetUniformLocation(gProgramId, "lightPosTwo");
    GLint viewPositionLocTwo = glGetUniformLocation(gProgramId, "viewPositionTwo");

    glUniform3f(lightColorLocTwo, gLightColorTwo.r, gLightColorTwo.g, gLightColorTwo.b);
    glUniform3f(lightPositionLocTwo, gLightPositionTwo.x, gLightPositionTwo.y, gLightPositionTwo.z);
    const glm::vec3 cameraPositionTwo = gCamera.Position;
    glUniform3f(viewPositionLocTwo, cameraPositionTwo.x, cameraPositionTwo.y, cameraPositionTwo.z);


    /**
       Desk affine transformations
    **/
    glBindVertexArray(gDeskMesh.vao);
    glUseProgram(gProgramId);

  
    model = setupTransform(
        glm::scale(glm::vec3(8.0f, 2.5f, 1.0f)),
        glm::rotate(15.0f, glm::vec3(2.0, 0.0f, 0.0f)),
        glm::translate(glm::vec3(0.0f, -0.65f, 0.5f))
    );



    view = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f)); // Moves the camera backwards -3 units in Z
    projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureDeskId);

    // Draw the desk
    glDrawElements(GL_TRIANGLES, gDeskMesh.nIndices, GL_UNSIGNED_SHORT, NULL);



    /**
      Desk Mat affine transformations
   **/
    glBindVertexArray(gDeskMatMesh.vao);
    glUseProgram(gProgramId);


    model = setupTransform(
        glm::scale(glm::vec3(4.5f, 2.0f, 1.0f)),
        glm::rotate(15.0f, glm::vec3(2.0, 0.0f, 0.0f)),
        glm::translate(glm::vec3(0.0f, -0.65f, 0.55f))
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureDeskMatId);

    // Draw the desk
    glDrawElements(GL_TRIANGLES, gDeskMatMesh.nIndices, GL_UNSIGNED_SHORT, NULL);

    /**
        Keyboard affine transformations
     **/

    glBindVertexArray(gKeyboardMesh.vao);
    glUseProgram(gProgramId);

    
    model = setupTransform(
        glm::scale(glm::vec3(2.5f, 0.80f, 1.0f)),
        glm::rotate(15.0f, glm::vec3(1.0, 0.0f, 0.0f)),
        glm::translate(glm::vec3(0.05f, -0.94f, 0.94f))
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Apply textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureKeyboardId);


    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gKeyboardMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    /**
        Golf Ball affine transformations
     **/
    glBindVertexArray(gGolfBallMesh.vao);
    glUseProgram(gProgramId);

    model = setupTransform(
        glm::scale(glm::vec3(.14f)),
        glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::translate(glm::vec3(0.70f, -0.20f, 0.6f))
    );

    // Set the model matrix uniform in the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureGolfBallId);

    glDrawElements(GL_TRIANGLES, gGolfBallMesh.nIndices, GL_UNSIGNED_SHORT, nullptr);

    /**
        Airpods affine transformations
     **/
    glBindVertexArray(gAirPodMesh.vao);
    glUseProgram(gProgramId);


    model = setupTransform(
        glm::scale(glm::vec3(0.40f, 0.45f, 0.15f)),
        glm::rotate(540.0f, glm::vec3(1.0, 0.0f, 0.0f)),
        glm::translate(glm::vec3(0.0f, -0.25f, 0.6f))
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureAirPodId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gAirPodMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    /**
        Glass affine transformations
     **/
    glBindVertexArray(gGlassMesh.vao);
    glUseProgram(gProgramId);

   model = setupTransform(
        glm::scale(glm::vec3(.30f, .50f, .30f)),
        glm::rotate(glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::translate(glm::vec3(-0.90f, 0.20f, 0.6f))
    );



   glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set the UV scale
    gUVScale = glm::vec2(1.0f, 1.0f);
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureGlassId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gGlassMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle */

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UKeyboardCreateMesh function
void UCreateKeyboardMesh(GLMesh& mesh)
{
    // Position, Color, and UV data
    GLfloat keyboardVerts[] =
    {
        // Vertex Positions   // Texture Coordinates    // Normals
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,               1.0f,  1.0f, 0.0f,  // Top Right Vertex 0
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,               1.0f,  1.0f, 0.0f, // Bottom Right Vertex 1
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,               1.0f,  1.0f, 0.0f, // Bottom Left Vertex 2
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,               1.0f,  1.0f, 0.0f  // Top Left Vertex 3
    };

    // Index data to share position data
    GLushort keyboardIndices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3   // Triangle 2
    };

    floatsPerVertex = 3;
    floatsPerUV = 2;    // 2 UV coordinates
    floatsPerNormal = 3;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activate the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(keyboardVerts), keyboardVerts, GL_STATIC_DRAW); // Send vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(keyboardIndices) / sizeof(keyboardIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(keyboardIndices), keyboardIndices, GL_STATIC_DRAW);

    // Strides between vertex coordinates, and UV coordinates
    GLint stride = sizeof(float) * (floatsPerVertex +  floatsPerUV + floatsPerNormal);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerUV)));
    glEnableVertexAttribArray(2);
}

// Implements the UGolfBallCreateMesh function
void UCreateGolfBallMesh(GLMesh& mesh)
{
    const int numSegments = 92;
    const int numStacks = 74;
    float textureScale = 1.0f;

    std::vector<GLfloat> golfBallVerts;
    std::vector<GLushort> golfBallIndices;

    // Generate vertices and indices for the golf ball
    for (int i = 0; i <= numStacks; ++i)
    {
        float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(numStacks);
        float phiNext = glm::pi<float>() * static_cast<float>(i + 1) / static_cast<float>(numStacks);

        // generate vertices
        for (int j = 0; j <= numSegments; ++j)
        {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(numSegments);

            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);

            float u = static_cast<float>(j) / static_cast<float>(numSegments);  // Calculate U (horizontal) texture coordinate
            float v = 1.0f - (static_cast<float>(i) / static_cast<float>(numStacks));  // Calculate V (vertical) texture coordinate


            golfBallVerts.push_back(x);
            golfBallVerts.push_back(y);
            golfBallVerts.push_back(z);
            golfBallVerts.push_back(u * textureScale);
            golfBallVerts.push_back(v * textureScale);
            golfBallVerts.push_back(1.0f);
            golfBallVerts.push_back(1.0f);
            golfBallVerts.push_back(0.0f);

        }
    }

    // generate indices
    for (int i = 0; i < numStacks; ++i)
    {
        for (int j = 0; j < numSegments; ++j)
        {
            int nextRow = i + 1;
            int nextColumn = (j + 1) % (numSegments + 1);

            int currIdx = i * (numSegments + 1) + j;
            int nextRowIdx = nextRow * (numSegments + 1) + j;
            int nextColumnIdx = i * (numSegments + 1) + nextColumn;
            int nextRowNextColumnIdx = nextRow * (numSegments + 1) + nextColumn;

            // triangle 1
            golfBallIndices.push_back(currIdx);
            golfBallIndices.push_back(nextRowIdx);
            golfBallIndices.push_back(nextColumnIdx);

            // triangle 2
            golfBallIndices.push_back(nextColumnIdx);
            golfBallIndices.push_back(nextRowIdx);
            golfBallIndices.push_back(nextRowNextColumnIdx);
        }
    }



    floatsPerVertex = 3;
    floatsPerUV = 2;    // 2 UV coordinates
    floatsPerNormal = 3;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, golfBallVerts.size() * sizeof(GLfloat), golfBallVerts.data(), GL_STATIC_DRAW);

    mesh.nIndices = golfBallIndices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, golfBallIndices.size() * sizeof(GLushort), golfBallIndices.data(), GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV + floatsPerNormal);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerUV)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateAirPodMesh function
void UCreateAirPodMesh(GLMesh& mesh)
{
    GLfloat airPodVerts[] =
    {
        // Vertex Positions  // Texture coordinates     // Normals
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f,                1.0f,  1.0f, 0.0f, // Top Right Vertex 0
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f,                1.0f,  1.0f, 0.0f,// Bottom Right Vertex 1
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,                1.0f,  1.0f, 0.0f, // Bottom Left Vertex 2
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,                1.0f,  1.0f, 0.0f,  // Top Left Vertex 3
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,                1.0f,  1.0f, 0.0f, // Top Right Vertex (bottom) 4
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,                1.0f,  1.0f, 0.0f, // Bottom Right Vertex (bottom) 5
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,                1.0f,  1.0f, 0.0f, // Bottom Left Vertex (bottom) 6
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,                1.0f,  1.0f, 0.0f, // Top Left Vertex (bottom) 7
    };

    // indicies for airpod
    GLushort airPodIndices[] = {
        // top face
        0, 1, 3,
        1, 2, 3,
        // Bottom face
        4, 5, 7,
        5, 6, 7,
        // Other faces (sides)
        0, 4, 1,
        1, 4, 5,
        1, 5, 2,
        2, 5, 6,
        2, 6, 3,
        3, 6, 7,
        3, 7, 0,
        0, 7, 4
    };


    floatsPerVertex = 3;
    floatsPerUV = 2;    // 2 UV coordinates
    floatsPerNormal = 3;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(airPodVerts), airPodVerts, GL_STATIC_DRAW);

    mesh.nIndices = sizeof(airPodIndices) / sizeof(airPodIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(airPodIndices), airPodIndices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV + floatsPerNormal);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerUV)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreatGlassMesh function
void UCreateGlassMesh(GLMesh& mesh)
{
    const int numSegments = 92;

    std::vector<GLfloat> glassVerts;
    std::vector<GLushort> glassIndices;
    const float cylinderHeight = 2.0f;
    const float radius = 1.0;

    // Create vertices for the top circle
    for (int i = 0; i <= numSegments; ++i) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(numSegments);
        float u = static_cast<float>(i) / static_cast<float>(numSegments); 
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);


        glassVerts.push_back(x);
        glassVerts.push_back(cylinderHeight / 2);
        glassVerts.push_back(z);
        glassVerts.push_back(u);
        glassVerts.push_back(0.0f);
        glassVerts.push_back(1.0f);
        glassVerts.push_back(1.0f);
        glassVerts.push_back(0.0f);


        // Bottom circle vertex with color
        glassVerts.push_back(x);
        glassVerts.push_back(-cylinderHeight / 2);
        glassVerts.push_back(z);
        glassVerts.push_back(u);
        glassVerts.push_back(1.0f);
        glassVerts.push_back(1.0f);
        glassVerts.push_back(1.0f);
        glassVerts.push_back(0.0f);

    }

    // Create indices for the cylinder
    for (int i = 0; i < numSegments; ++i) {
        int top = i * 2;
        int bottom = top + 1;
        int nextTop = ((i + 1) % numSegments) * 2;
        int nextBottom = nextTop + 1;

        // Create two triangles for each segment
        glassIndices.push_back(top);
        glassIndices.push_back(nextTop);
        glassIndices.push_back(bottom);

        glassIndices.push_back(bottom);
        glassIndices.push_back(nextTop);
        glassIndices.push_back(nextBottom);
    }

    floatsPerVertex = 3;
    floatsPerUV = 2;
    floatsPerNormal = 3;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, glassVerts.size() * sizeof(GLfloat), &glassVerts[0], GL_STATIC_DRAW);

    mesh.nIndices = glassIndices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, glassIndices.size() * sizeof(GLushort), &glassIndices[0], GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV + floatsPerNormal);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerUV)));
    glEnableVertexAttribArray(2);
}

// Implements the UKeyboardCreateMesh function
void UCreateDeskMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat deskVerts[] =
    {
        // Vertex Positions   // Texture Coordinates        // Normals
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,                   1.0f,  1.0f, 0.0f, // Top Right Vertex 0
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,                   1.0f,  1.0f, 0.0f, // Bottom Right Vertex 1
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,                   1.0f,  1.0f, 0.0f, // Bottom Left Vertex 2
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,                   1.0f,  1.0f, 0.0f, // Top Left Vertex 3
    };

    // Index data to share position data
    GLushort deskIndices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3   // Triangle 2
    };

    floatsPerVertex = 3;
    floatsPerUV = 2;
    floatsPerNormal = 3;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(deskVerts), deskVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(deskIndices) / sizeof(deskIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(deskIndices), deskIndices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV + floatsPerNormal);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerUV)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId, bool flipImg)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {

        if (flipImg) {
            flipImageVertically(image, width, height, channels);
        }
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
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

// Mouse position event handler 
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
   // ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseButtonCallback(GLFWwindow*, int button, int action, int mods)
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



void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

void UDestroyProgram()
{
    // Release mesh data
    UDestroyMesh(gKeyboardMesh);
    UDestroyMesh(gGolfBallMesh);
    UDestroyMesh(gAirPodMesh);
    UDestroyMesh(gGlassMesh);
    UDestroyMesh(gDeskMesh);
    UDestroyMesh(gDeskMatMesh);

    // Release texture
    UDestroyTexture(gTextureGolfBallId);
    UDestroyTexture(gTextureAirPodId);
    UDestroyTexture(gTextureKeyboardId);
    UDestroyTexture(gTextureDeskId);
    UDestroyTexture(gTextureDeskMatId);
    UDestroyTexture(gTextureGlassId);


    // Release shader program
    UDestroyShaderProgram(gProgramId);

}
