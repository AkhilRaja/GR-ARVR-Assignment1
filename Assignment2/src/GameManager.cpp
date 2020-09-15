//
//  GameManager.cpp
//  graphics_asgn1
//
//  Created by Akhil Raja on 15/09/20.
//

#include "GameManager.hpp"
#include "Vertices.h"


#define Updates_Per_Second 60.0f

// Build and compile our shader program
Shader lightingShader;
Shader lampShader;


// Function prototypes
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement( );

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos( 1.2f, 1.0f, 2.0f );

// Deltatime
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame


GLuint lightVAO;
glm::mat4 view;
glm::mat4 projection;
GLuint diffuseMap, specularMap, emissionMap;
GLuint VBO, boxVAO;


//Initialize the GameManager
GameManager::GameManager(bool running):
_running(running), _window(glfwGetCurrentContext())
{
    std::cout<<"Game Manager Constructor Called";
}

//Destructor
GameManager::~GameManager()
{
}

GameManager& GameManager::getGameManager()
{
    static GameManager *gameManager = NULL;
    
    if (gameManager == NULL) {
        
        // Init GLFW
        glfwInit( );
        // Set all the required options for GLFW
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
        
        // Create a GLFWwindow object that we can use for GLFW's functions
        GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr );
        
        if ( nullptr == window )
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate( );
            
//            return EXIT_FAILURE;
        }
        
        glfwMakeContextCurrent( window );
        glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
        
        // Set the required callback functions
        glfwSetKeyCallback( window, KeyCallback );
        glfwSetCursorPosCallback( window, MouseCallback );
        
        // GLFW Options
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        
        gameManager = new GameManager(true);
        std::cout << "GameManager created" << std::endl;
        
        // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
        glewExperimental = GL_TRUE;
        // Initialize GLEW to setup the OpenGL Function pointers
        if ( GLEW_OK != glewInit( ) )
        {
            std::cout << "Failed to initialize GLEW" << std::endl;
//            return EXIT_FAILURE;
        }
        
        // Define the viewport dimensions
        glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
        
        // OpenGL options
        glEnable( GL_DEPTH_TEST );
        
        // Build and compile our shader program
        lightingShader.intialize( "/Users/akhilraja/Downloads/2D Maze Runner/src/res/shaders/lighting.vs", "/Users/akhilraja/Downloads/2D Maze Runner/src/res/shaders/lighting.frag" );
        lampShader.intialize( "/Users/akhilraja/Downloads/2D Maze Runner/src/res/shaders/lamp.vs", "/Users/akhilraja/Downloads/2D Maze Runner/src/res/shaders/lamp.frag" );
        
        
        
        // First, set the container's VAO (and VBO)
        
        glGenVertexArrays( 1, &boxVAO );
        glGenBuffers( 1, &VBO );
        
        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
        
        glBindVertexArray( boxVAO );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid * )0 );
        glEnableVertexAttribArray(0);
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid * )( 3 * sizeof( GLfloat ) ) );
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid * )( 6 * sizeof( GLfloat ) ) );
        glEnableVertexAttribArray( 2 );
        glBindVertexArray( 0 );
        
        // Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))

        glGenVertexArrays( 1, &lightVAO );
        glBindVertexArray( lightVAO );
        // We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        // Set the vertex attributes (only position data for the lamp))
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ), ( GLvoid * )0 ); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
        glEnableVertexAttribArray( 0 );
        glBindVertexArray( 0 );
        
        // Load textures
        
        glGenTextures( 1, &diffuseMap );
        glGenTextures( 1, &specularMap );
        glGenTextures( 1, &emissionMap );
        
        int imageWidth, imageHeight;
        unsigned char *image;
        
        // Diffuse map
        image = SOIL_load_image( "/Users/akhilraja/Downloads/2D Maze Runner/src/res/images/container2.png", &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB );
        glBindTexture( GL_TEXTURE_2D, diffuseMap );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
        glGenerateMipmap( GL_TEXTURE_2D );
        SOIL_free_image_data( image );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        
        // Specular map
        image = SOIL_load_image( "/Users/akhilraja/Downloads/2D Maze Runner/src/res/images/container2_specular.png", &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB );
        glBindTexture( GL_TEXTURE_2D, specularMap );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
        glGenerateMipmap( GL_TEXTURE_2D );
        SOIL_free_image_data( image );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        glBindTexture( GL_TEXTURE_2D, 0 );
        
        
        // Set texture units
        lightingShader.Use( );
        glUniform1i( glGetUniformLocation( lightingShader.Program, "material.diffuse" ), 0 );
        glUniform1i( glGetUniformLocation( lightingShader.Program, "material.specular" ), 1 );
        
        projection = glm::perspective( camera.GetZoom( ), ( GLfloat )SCREEN_WIDTH / ( GLfloat )SCREEN_HEIGHT, 0.1f, 100.0f );
        
    }
    
    return *gameManager;
}

void GameManager::runGameLoop()
{
    double lastTime = glfwGetTime();
    
    //Update Function
    while (_running) {
        
        double currentTime = glfwGetTime();
        deltaTime += (currentTime - lastTime) * Updates_Per_Second;
        lastTime = currentTime;
        
        while (deltaTime >= 1.0f) {
            
            _running = !glfwWindowShouldClose(_window);
            //Do stuff
            
            // Calculate deltatime of current frame
                    GLfloat currentFrame = glfwGetTime( );
                    deltaTime = currentFrame - lastFrame;
                    lastFrame = currentFrame;
                    
                    // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
                    glfwPollEvents( );
                    DoMovement( );
                    
                    // Clear the colorbuffer
                    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
                    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                    
                    
                    // Use cooresponding shader when setting uniforms/drawing objects
                    lightingShader.Use( );
                    GLint viewPosLoc = glGetUniformLocation( lightingShader.Program, "viewPos" );
                    glUniform3f( viewPosLoc, camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z);
                    // Set material properties
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "material.shininess" ), 32.0f );
                    // == ==========================
                    // Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
                    // the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
                    // by defining light types as classes and set their values in there, or by using a more efficient uniform approach
                    // by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
                    // == ==========================
                    // Directional light
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "dirLight.direction" ), -0.2f, -1.0f, -0.3f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "dirLight.ambient" ), 0.05f, 0.05f, 0.05f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "dirLight.diffuse" ), 0.4f, 0.4f, 0.4f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "dirLight.specular" ), 0.5f, 0.5f, 0.5f );
                    
                    // Point light 1
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[0].position" ), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[0].ambient" ), 0.05f, 0.05f, 0.05f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[0].diffuse" ), 0.8f, 0.8f, 0.8f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[0].specular" ), 1.0f, 1.0f, 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[0].constant" ), 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[0].linear" ), 0.09f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[0].quadratic" ), 0.032f );
                    
                    // Point light 2
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[1].position" ), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[1].ambient" ), 0.05f, 0.05f, 0.05f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[1].diffuse" ), 0.8f, 0.8f, 0.8f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[1].specular" ), 1.0f, 1.0f, 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[1].constant" ), 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[1].linear" ), 0.09f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[1].quadratic" ), 0.032f );
                    
                    // Point light 3
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[2].position" ), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[2].ambient" ), 0.05f, 0.05f, 0.05f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[2].diffuse" ), 0.8f, 0.8f, 0.8f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[2].specular" ), 1.0f, 1.0f, 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[2].constant" ), 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[2].linear" ), 0.09f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[2].quadratic" ), 0.032f );
                    
                    // Point light 4
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[3].position" ), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[3].ambient" ), 0.05f, 0.05f, 0.05f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[3].diffuse" ), 0.8f, 0.8f, 0.8f );
                    glUniform3f( glGetUniformLocation( lightingShader.Program, "pointLights[3].specular" ), 1.0f, 1.0f, 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[3].constant" ), 1.0f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[3].linear" ), 0.09f );
                    glUniform1f( glGetUniformLocation( lightingShader.Program, "pointLights[3].quadratic" ), 0.032f );
                    
            //        // SpotLight
            //        glUniform3f( glGetUniformLocation( lightingShader.Program, "spotLight.position" ), camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
            //        glUniform3f( glGetUniformLocation( lightingShader.Program, "spotLight.direction" ), camera.GetFront( ).x, camera.GetFront( ).y, camera.GetFront( ).z );
            //        glUniform3f( glGetUniformLocation( lightingShader.Program, "spotLight.ambient" ), 0.0f, 0.0f, 0.0f );
            //        glUniform3f( glGetUniformLocation( lightingShader.Program, "spotLight.diffuse" ), 1.0f, 1.0f, 1.0f );
            //        glUniform3f( glGetUniformLocation( lightingShader.Program, "spotLight.specular" ), 1.0f, 1.0f, 1.0f );
            //        glUniform1f( glGetUniformLocation( lightingShader.Program, "spotLight.constant" ), 1.0f );
            //        glUniform1f( glGetUniformLocation( lightingShader.Program, "spotLight.linear" ), 0.09f );
            //        glUniform1f( glGetUniformLocation( lightingShader.Program, "spotLight.quadratic" ), 0.032f );
            //        glUniform1f( glGetUniformLocation( lightingShader.Program, "spotLight.cutOff" ), glm::cos( glm::radians( 12.5f ) ) );
            //        glUniform1f( glGetUniformLocation( lightingShader.Program, "spotLight.outerCutOff" ), glm::cos( glm::radians( 15.0f ) ) );
            //
                    // Create camera transformations
                    
                    view = camera.GetViewMatrix( );

                    // Get the uniform locations
                    GLint modelLoc = glGetUniformLocation( lightingShader.Program, "model" );
                    GLint viewLoc = glGetUniformLocation( lightingShader.Program, "view" );
                    GLint projLoc = glGetUniformLocation( lightingShader.Program, "projection" );
                    // Pass the matrices to the shader
                    glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
                    glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
                    
                    // Bind diffuse map
                    glActiveTexture( GL_TEXTURE0 );
                    glBindTexture( GL_TEXTURE_2D, diffuseMap );
                    // Bind specular map
                    glActiveTexture( GL_TEXTURE1 );
                    glBindTexture( GL_TEXTURE_2D, specularMap );
                    
                    // Draw 10 containers with the same VAO and VBO information; only their world space coordinates differ
                    glm::mat4 model;
                    glBindVertexArray( boxVAO );
                    for ( GLuint i = 0; i < 10; i++ )
                    {
                        model = glm::mat4( );
                        model = glm::translate( model, cubePositions[i] );
                        GLfloat angle = 20.0f * i;
                        model = glm::rotate( model, angle, glm::vec3( 1.0f, 0.3f, 0.5f ) );
                        glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
                        
                        glDrawArrays( GL_TRIANGLES, 0, 36 );
                    }
                    glBindVertexArray( 0 );
                    
                    
                    // Also draw the lamp object, again binding the appropriate shader
                    lampShader.Use( );
                    // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
                    modelLoc = glGetUniformLocation( lampShader.Program, "model" );
                    viewLoc = glGetUniformLocation( lampShader.Program, "view" );
                    projLoc = glGetUniformLocation( lampShader.Program, "projection" );
                    // Set matrices
                    glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
                    glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
                    model = glm::mat4( );
                    model = glm::translate( model, lightPos );
                    model = glm::scale( model, glm::vec3( 0.2f ) ); // Make it a smaller cube
                    glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
                    // Draw the light object (using light's vertex attributes)
                    glBindVertexArray( lightVAO );
                    glDrawArrays( GL_TRIANGLES, 0, 36 );
                    glBindVertexArray( 0 );
                    
                    
                    // We now draw as many light bulbs as we have point lights.
                    glBindVertexArray( lightVAO );
                    for ( GLuint i = 0; i < 4; i++ )
                    {
                        model = glm::mat4( );
                        model = glm::translate( model, pointLightPositions[i] );
                        model = glm::scale( model, glm::vec3( 0.2f ) ); // Make it a smaller cube
                        glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
                        glDrawArrays( GL_TRIANGLES, 0, 36 );
                    }
                    glBindVertexArray( 0 );
                    
                    // Swap the screen buffers
            glfwSwapBuffers( _window );
            
            
            // Update Time
            --deltaTime;
        }
    }
}

void GameManager::destroyGameManager()
{
    GameManager *gameManager = &getGameManager();
    delete gameManager;
    
    std::cout << "GameManager destroyed" << std::endl;
    
    glDeleteVertexArrays( 1, &boxVAO );
    glDeleteVertexArrays( 1, &lightVAO );
    glDeleteBuffers( 1, &VBO );
              
      
    GLFWwindow *window = glfwGetCurrentContext();
    glfwDestroyWindow(window);
    
    glfwTerminate();
    
}

// Moves/alters the camera positions based on user input
void DoMovement( )
{
    // Camera controls
    if ( keys[GLFW_KEY_W] || keys[GLFW_KEY_UP] )
    {
        camera.ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN] )
    {
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT] )
    {
        camera.ProcessKeyboard( LEFT, deltaTime );
    }
    
    if ( keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT] )
    {
        camera.ProcessKeyboard( RIGHT, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
//    std::cout<<"Key Callback Called";
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if ( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if ( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

void MouseCallback( GLFWwindow *window, double xPos, double yPos )
{
    if ( firstMouse )
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement( xOffset, yOffset );
}