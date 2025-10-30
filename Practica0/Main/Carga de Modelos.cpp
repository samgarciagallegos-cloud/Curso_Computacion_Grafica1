// Std. Includes
#include <string>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "stb_image.h"

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
GLuint LoadTexture(const char* path);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Función para cargar texturas
GLuint LoadTexture(const char* path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Configurar parámetros de textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Cargar imagen
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;
        else if (nrChannels == 1)
            format = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Textura cargada exitosamente: " << path << std::endl;
    }
    else
    {
        std::cout << "Error al cargar la textura: " << path << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modelos 3D con Texturas", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Setup shader
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Cargar modelos
    Model Tazones2((char*)"Models/Tazones2.obj");
    Model Tejado((char*)"Models/Tejado.obj");
    Model TejadoMini((char*)"Models/TejadoMini.obj");
    Model TejadoMiniPiso2((char*)"Models/TejadoMiniPiso2.obj");
    Model TuboFuera((char*)"Models/TuboFuera.obj");
    Model TuboGas((char*)"Models/TuboGas.obj");
    Model VentanaSegundoPiso((char*)"Models/VentanaSegundoPiso.obj");
    Model BolsaHarinaAbierta((char*)"Models/BolsaHarinaAbierta.obj");
    Model BolsasHarina((char*)"Models/BolsasHarina.obj");
    Model BoteBasura((char*)"Models/BoteBasura.obj");
    Model CadenaLinterna((char*)"Models/CadenaLinterna.obj");
    Model Caja((char*)"Models/Caja.obj");
    Model Cajas2((char*)"Models/Cajas2.obj");
    Model CajasMadera((char*)"Models/CajasMadera.obj");
    Model Campana((char*)"Models/Campana.obj");
    Model Cortinas((char*)"Models/Cortinas.obj");
    Model Cubeta((char*)"Models/Cubeta.obj");
    Model Cuchillo((char*)"Models/Cuchillo.obj");
    Model Electricidad((char*)"Models/Electricidad.obj");
    Model Escaleras((char*)"Models/Escaleras.obj");
    Model Estufa((char*)"Models/Estufa.obj");
    Model Fregadero((char*)"Models/Fregadero.obj");
    Model Gas((char*)"Models/Gas.obj");
    Model Jalador((char*)"Models/Jalador.obj");
    Model Lamparas((char*)"Models/Lamparas.obj");
    Model Librero((char*)"Models/Librero.obj");
    Model LinternaLocal((char*)"Models/LinternaLocal.obj");
    Model Mesa((char*)"Models/Mesa.obj");
    Model olla((char*)"Models/olla.obj");
    Model olla1((char*)"Models/olla1.obj");
    Model Platos((char*)"Models/Platos.obj");
    Model PrimerPiso((char*)"Models/PrimerPiso.obj");
    Model Puertas((char*)"Models/Puertas.obj");
    Model Refrigerador((char*)"Models/Refrigerador.obj");
    Model Refrigerador1((char*)"Models/Refrigerador1.obj");
    Model TapeteAlmacen((char*)"Models/TapeteAlmacen.obj");
    Model TapeteCocina((char*)"Models/TapeteCocina.obj");
    Model Tazon((char*)"Models/Tazon.obj");

    // Cargar texturas
    GLuint textureTazones2 = LoadTexture("images/tazones2_texture.jpg");
    GLuint textureTejado = LoadTexture("images/tejado_texture.jpg");
    GLuint textureTejadoMini = LoadTexture("images/tejadoMini_texture.jpg");
    GLuint textureTejadoMiniPiso2 = LoadTexture("images/tejadoMiniPiso2_texture.jpg");
    GLuint textureTuboFuera = LoadTexture("images/tuboFuera_texture.jpg");
    GLuint textureTuboGas = LoadTexture("images/tuboGas_texture.jpg");
    GLuint textureVentanaSegundoPiso = LoadTexture("images/ventanaSegundoPiso_texture.jpg");
    GLuint textureBolsaHarinaAbierta = LoadTexture("images/bolsaHarinaAbierta_texture.jpg");
    GLuint textureBolsasHarina = LoadTexture("images/bolsasHarina_texture.jpg");
    GLuint textureBoteBasura = LoadTexture("images/boteBasura_texture.jpg");
    GLuint textureCadenaLinterna = LoadTexture("images/cadenaLinterna_texture.jpg");
    GLuint textureCaja = LoadTexture("images/caja_texture.jpg");
    GLuint textureCajas2 = LoadTexture("images/cajas2_texture.jpg");
    GLuint textureCajasMadera = LoadTexture("images/cajasMadera_texture.jpg");
    GLuint textureCampana = LoadTexture("images/campana_texture.jpg");
    GLuint textureCortinas = LoadTexture("images/cortinas_texture.jpg");
    GLuint textureCubeta = LoadTexture("images/cubeta_texture.jpg");
    GLuint textureCuchillo = LoadTexture("images/cuchillo_texture.jpg");
    GLuint textureElectricidad = LoadTexture("images/electricidad_texture.jpg");
    GLuint textureEscaleras = LoadTexture("images/escaleras_texture.jpg");
    GLuint textureEstufa = LoadTexture("images/estufa_texture.jpg");
    GLuint textureFregadero = LoadTexture("images/fregadero_texture.jpg");
    GLuint textureGas = LoadTexture("images/gas_texture.jpg");
    GLuint textureJalador = LoadTexture("images/jalador_texture.jpg");
    GLuint textureLamparas = LoadTexture("images/lamparas_texture.jpg");
    GLuint textureLibrero = LoadTexture("images/librero_texture.jpg");
    GLuint textureLinternaLocal = LoadTexture("images/linternaLocal_texture.jpg");
    GLuint textureMesa = LoadTexture("images/mesa_texture.jpg");
    GLuint textureOlla = LoadTexture("images/olla_texture.jpg");
    GLuint textureOlla1 = LoadTexture("images/olla1_texture.jpg");
    GLuint texturePlatos = LoadTexture("images/platos_texture.jpg");
    GLuint texturePrimerPiso = LoadTexture("images/primerPiso_texture.jpg");
    GLuint texturePuertas = LoadTexture("images/puertas_texture.jpg");
    GLuint textureRefrigerador = LoadTexture("images/refrigerador_texture.jpg");
    GLuint textureRefrigerador1 = LoadTexture("images/refrigerador1_texture.jpg");
    GLuint textureTapeteAlmacen = LoadTexture("images/tapeteAlmacen_texture.jpg");
    GLuint textureTapeteCocina = LoadTexture("images/tapeteCocina_texture.jpg");
    GLuint textureTazon = LoadTexture("images/tazon_texture.jpg");


    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Renderizar Tazones2
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTazones2);
        glm::mat4 model(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Tazones2.Draw(shader);

        // Renderizar Tejado
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTejado);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Tejado.Draw(shader);

        // Renderizar TejadoMini
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTejadoMini);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TejadoMini.Draw(shader);

        // Renderizar TejadoMiniPiso2
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTejadoMiniPiso2);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TejadoMiniPiso2.Draw(shader);

        // Renderizar TuboFuera
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTuboFuera);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TuboFuera.Draw(shader);

        // Renderizar TuboGas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTuboGas);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TuboGas.Draw(shader);

        // Renderizar VentanaSegundoPiso
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureVentanaSegundoPiso);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        VentanaSegundoPiso.Draw(shader);

        // Renderizar BolsaHarinaAbierta
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureBolsaHarinaAbierta);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        BolsaHarinaAbierta.Draw(shader);

        // Renderizar BolsasHarina
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureBolsasHarina);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        BolsasHarina.Draw(shader);

        // Renderizar BoteBasura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureBoteBasura);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        BoteBasura.Draw(shader);

        // Renderizar CadenaLinterna
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCadenaLinterna);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        CadenaLinterna.Draw(shader);

        // Renderizar Caja
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCaja);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Caja.Draw(shader);

        // Renderizar Cajas2
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCajas2);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Cajas2.Draw(shader);

        // Renderizar CajasMadera
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCajasMadera);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        CajasMadera.Draw(shader);

        // Renderizar Campana
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCampana);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Campana.Draw(shader);

        // Renderizar Cortinas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCortinas);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Cortinas.Draw(shader);

        // Renderizar Cubeta
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCubeta);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Cubeta.Draw(shader);

        // Renderizar Cuchillo
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureCuchillo);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Cuchillo.Draw(shader);

        // Renderizar Electricidad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureElectricidad);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Electricidad.Draw(shader);

        // Renderizar Escaleras
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureEscaleras);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Escaleras.Draw(shader);

        // Renderizar Estufa
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureEstufa);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Estufa.Draw(shader);

        // Renderizar Fregadero
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureFregadero);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Fregadero.Draw(shader);

        // Renderizar Gas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureGas);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Gas.Draw(shader);

        // Renderizar Jalador
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureJalador);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Jalador.Draw(shader);

        // Renderizar Lamparas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureLamparas);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Lamparas.Draw(shader);

        // Renderizar Librero
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureLibrero);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Librero.Draw(shader);

        // Renderizar LinternaLocal
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureLinternaLocal);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        LinternaLocal.Draw(shader);

        // Renderizar Mesa
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureMesa);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Mesa.Draw(shader);

        // Renderizar olla
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureOlla);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        olla.Draw(shader);

        // Renderizar olla1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureOlla1);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        olla1.Draw(shader);

        // Renderizar Platos
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePlatos);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Platos.Draw(shader);

        // Renderizar PrimerPiso
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePrimerPiso);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        PrimerPiso.Draw(shader);

        // Renderizar Puertas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePuertas);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Puertas.Draw(shader);

        // Renderizar Refrigerador
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureRefrigerador);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Refrigerador.Draw(shader);

        // Renderizar Refrigerador1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureRefrigerador1);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Refrigerador1.Draw(shader);

        // Renderizar TapeteAlmacen
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTapeteAlmacen);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TapeteAlmacen.Draw(shader);

        // Renderizar TapeteCocina
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTapeteCocina);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        TapeteCocina.Draw(shader);

        // Renderizar Tazon
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTazon);
        model = glm::mat4(1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Tazon.Draw(shader);


        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}