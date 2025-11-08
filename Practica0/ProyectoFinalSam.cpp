#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <ctime>
#include <cstdlib>

// Constante PI
const float PI = 3.14159265359f;
// Constantes
const GLuint WINDOW_WIDTH = 800;
const GLuint WINDOW_HEIGHT = 600;

// Variables globales para cámara
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WINDOW_WIDTH / 2.0f;
GLfloat lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
bool keys[1024] = { false };

// Variables para animaciones del modelo
float puertaRefrigerador = 0.0f;
bool animandoPuerta = false;
float puertaDestino = 0.0f;

// Variables para animación de banderines
float banderinesInclinacion = 0.0f;
bool animandoBanderines = false;
float banderinesDestino = 0.0f;
const float banderinesMaxInclinacion = 25.0f; // Grados máximos hacia adelante/atrás
// Sistema de iluminación
bool lightStates[4] = { false, false, false, true }; // Estados de las 3 lámparas + Luna

// Posiciones de las luces puntuales
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f, 3.0f, -2.0f),    // LamparasCuarto2
    glm::vec3(0.0f, 3.0f, 2.0f),     // LamparasCuarto1
    glm::vec3(-2.9f, 2.0f, 5.0f),     // LinternaLocal
    glm::vec3(4.0f, 8.0f, 5.0f)      // Luna
};

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat currentFrame = 0.0f;
// Variables para animación de agua del grifo (AGREGAR)
bool aguaCayendo = false;
struct Gota {
    glm::vec3 posicion;
    float velocidad;
    bool activa;
};
std::vector<Gota> gotas;
const int MAX_GOTAS = 50;
glm::vec3 posicionGrifo = glm::vec3(1.97f, 1.3f, 1.77f); // Ajusta según tu fregadero
float tiempoUltimaGota = 0.0f;
const float intervaloGotas = 0.02f;
// Variables para fuegos artificiales
bool fuegosActivos = false;
struct Particula {
    glm::vec3 posicion;
    glm::vec3 velocidad;
    glm::vec3 color;
    float vida;
    float vidaInicial;
    float tamano;
    bool activa;
};
std::vector<Particula> particulas;
const int MAX_PARTICULAS = 1000;
float tiempoUltimoFuego = 0.0f;
const float intervaloFuegos = 1.5f;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ProcessMovement();

GLuint LoadTexture(const char* path)
{
    GLuint textureID = 0;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, nrChannels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;
        else if (nrChannels == 1)
            format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Textura cargada: " << path << std::endl;
    }
    else
    {
        std::cout << "Error al cargar: " << path << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
// Estructura para geometría simple
struct SimpleGeometry {
    GLuint VAO, VBO, EBO;
    GLuint texture;
    GLuint indexCount;

    SimpleGeometry() : VAO(0), VBO(0), EBO(0), texture(0), indexCount(0) {}

    void Draw() {
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~SimpleGeometry() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

// Función para crear un círculo (reloj)
SimpleGeometry* CreateCircle(GLuint texture, float radius, int segments) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    // Centro del círculo
    vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f });

    // Vértices del perímetro
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * PI;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        float u = (cos(angle) + 1.0f) * 0.5f;
        float v = (sin(angle) + 1.0f) * 0.5f;

        vertices.insert(vertices.end(), { x, y, 0.0f, 0.0f, 0.0f, 1.0f, u, v });
    }

    // Índices
    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    geo->indexCount = (GLuint)indices.size();

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear una manecilla del reloj
SimpleGeometry* CreateClockHand(GLuint texture, float width, float length) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Posición              Normal              TexCoords
        -width / 2, 0.0f, 0.01f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         width / 2, 0.0f, 0.01f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         width / 2, length, 0.01f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -width / 2, length, 0.01f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f
    };

    std::vector<GLuint> indices = { 0, 1, 2, 2, 3, 0 };
    geo->indexCount = 6;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear un cubo
SimpleGeometry* CreateCube(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Cara frontal (z+)
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Cara trasera (z-)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

         // Cara superior (y+)
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

          // Cara inferior (y-)
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,

          // Cara derecha (x+)
           0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
           0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
           0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

           // Cara izquierda (x-)
           -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
           -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,    // Frontal
        4, 5, 6,  6, 7, 4,    // Trasera
        8, 9, 10, 10, 11, 8,  // Superior
        12, 13, 14, 14, 15, 12, // Inferior
        16, 17, 18, 18, 19, 16, // Derecha
        20, 21, 22, 22, 23, 20  // Izquierda
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear superficie de mesa
SimpleGeometry* CreateMesaTop(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Superficie superior
        -0.5f, 0.05f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.5f, 0.05f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.05f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.5f, 0.05f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Superficie inferior
        -0.5f, 0.0f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Borde frontal (Z-)
        -0.5f, 0.0f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.5f, 0.05f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, 0.05f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

        // Borde trasero (Z+)
        -0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f, 0.05f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f, 0.05f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Borde derecho (X+)
         0.5f, 0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.05f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.5f, 0.05f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Borde izquierdo (X-)
         -0.5f, 0.0f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.5f, 0.0f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.5f, 0.05f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.5f, 0.05f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,      // Superior
        4, 6, 5,  4, 7, 6,      // Inferior
        8, 9, 10, 10, 11, 8,    // Borde frontal
        12, 14, 13, 12, 15, 14, // Borde trasero
        16, 17, 18, 18, 19, 16, // Borde derecho
        20, 22, 21, 20, 23, 22  // Borde izquierdo
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
// Función para crear pata de mesa
SimpleGeometry* CreatePataMesa(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Cilindro simplificado como cubo delgado
        -0.05f, -0.7f, -0.05f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         0.05f, -0.7f, -0.05f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.05f, -0.7f,  0.05f,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.05f, -0.7f,  0.05f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.05f, 0.0f, -0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.05f, 0.0f, -0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.05f, 0.0f,  0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.05f, 0.0f,  0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,  // Abajo
        4, 6, 5,  4, 7, 6,  // Arriba
        0, 1, 5,  5, 4, 0,  // Laterales
        1, 2, 6,  6, 5, 1,
        2, 3, 7,  7, 6, 2,
        3, 0, 4,  4, 7, 3
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
// Función para crear Narutomaki (cilindro pequeño)
SimpleGeometry* CreateNarutomaki(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Base inferior (y = 0.0f)
        -0.05f, 0.0f, -0.05f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.05f, 0.0f, -0.05f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.05f, 0.0f,  0.05f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.05f, 0.0f,  0.05f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Base superior (y = 0.05f)
        -0.05f, 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.05f, 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.05f, 0.05f,  0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.05f, 0.05f,  0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Lado frontal (Z+)
        -0.05f, 0.0f,  0.05f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.05f, 0.0f,  0.05f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.05f, 0.05f,  0.05f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.05f, 0.05f,  0.05f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Lado trasero (Z-)
        -0.05f, 0.0f, -0.05f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.05f, 0.0f, -0.05f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.05f, 0.05f, -0.05f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -0.05f, 0.05f, -0.05f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        // Lado derecho (X+)
         0.05f, 0.0f, -0.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.05f, 0.0f,  0.05f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.05f, 0.05f,  0.05f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.05f, 0.05f, -0.05f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Lado izquierdo (X-)
         -0.05f, 0.0f, -0.05f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.05f, 0.0f,  0.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.05f, 0.05f,  0.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.05f, 0.05f, -0.05f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,      // Base inferior
        4, 6, 5,  4, 7, 6,      // Base superior
        8, 9, 10, 10, 11, 8,    // Frontal
        12, 14, 13, 12, 15, 14, // Trasero
        16, 17, 18, 18, 19, 16, // Derecho
        20, 22, 21, 20, 23, 22  // Izquierdo
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear Sartén
SimpleGeometry* CreateSarten(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Base inferior
        -0.2f, 0.0f, -0.2f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.0f, -0.2f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.0f,  0.2f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.2f, 0.0f,  0.2f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Base superior
        -0.2f, 0.02f, -0.2f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.02f, -0.2f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.2f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.2f, 0.02f,  0.2f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Lado frontal
        -0.2f, 0.0f,  0.2f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.2f, 0.0f,  0.2f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.2f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.2f, 0.02f,  0.2f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Lado trasero
        -0.2f, 0.0f, -0.2f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.2f, 0.0f, -0.2f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.2f, 0.02f, -0.2f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -0.2f, 0.02f, -0.2f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        // Lado derecho
         0.2f, 0.0f, -0.2f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.0f,  0.2f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.2f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.2f, 0.02f, -0.2f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Lado izquierdo
         -0.2f, 0.0f, -0.2f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.2f, 0.0f,  0.2f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.2f, 0.02f,  0.2f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.2f, 0.02f, -0.2f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  4, 7, 6,
        8, 9, 10, 10, 11, 8,
        12, 14, 13, 12, 15, 14,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 20, 23, 22
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear Tabla de Picar
SimpleGeometry* CreateTablaPicar(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Base inferior
        -0.2f, 0.0f, -0.15f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.0f, -0.15f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.0f,  0.15f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.2f, 0.0f,  0.15f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Base superior
        -0.2f, 0.02f, -0.15f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.02f, -0.15f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.15f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.2f, 0.02f,  0.15f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Lado frontal
        -0.2f, 0.0f,  0.15f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.2f, 0.0f,  0.15f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.15f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.2f, 0.02f,  0.15f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Lado trasero
        -0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.2f, 0.02f, -0.15f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -0.2f, 0.02f, -0.15f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        // Lado derecho
         0.2f, 0.0f, -0.15f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.2f, 0.0f,  0.15f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.2f, 0.02f,  0.15f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.2f, 0.02f, -0.15f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Lado izquierdo
         -0.2f, 0.0f, -0.15f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.2f, 0.0f,  0.15f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.2f, 0.02f,  0.15f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.2f, 0.02f, -0.15f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  4, 7, 6,
        8, 9, 10, 10, 11, 8,
        12, 14, 13, 12, 15, 14,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 20, 23, 22
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
// Función para crear hoja de cuchillo (rectángulo delgado alargado)
SimpleGeometry* CreateHojaCuchillo(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Base inferior
        -0.025f, 0.0f, -0.2f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.0f, -0.2f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.0f,  0.2f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.025f, 0.0f,  0.2f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Base superior
        -0.025f, 0.02f, -0.2f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.02f, -0.2f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.2f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.025f, 0.02f,  0.2f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Lado frontal (Z+)
        -0.025f, 0.0f,  0.2f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.025f, 0.0f,  0.2f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.2f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.025f, 0.02f,  0.2f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Lado trasero (Z-)
        -0.025f, 0.0f, -0.2f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.025f, 0.0f, -0.2f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.025f, 0.02f, -0.2f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -0.025f, 0.02f, -0.2f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        // Lado derecho (X+)
         0.025f, 0.0f, -0.2f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.0f,  0.2f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.2f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.025f, 0.02f, -0.2f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Lado izquierdo (X-)
         -0.025f, 0.0f, -0.2f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.025f, 0.0f,  0.2f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.025f, 0.02f,  0.2f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.025f, 0.02f, -0.2f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,      // Base inferior
        4, 6, 5,  4, 7, 6,      // Base superior
        8, 9, 10, 10, 11, 8,    // Frontal
        12, 14, 13, 12, 15, 14, // Trasero
        16, 17, 18, 18, 19, 16, // Derecho
        20, 22, 21, 20, 23, 22  // Izquierdo
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}

// Función para crear mango de cuchillo (cubo más pequeño)
SimpleGeometry* CreateMangoCuchillo(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Base inferior
        -0.025f, 0.0f, -0.05f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.0f, -0.05f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.0f,  0.05f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -0.025f, 0.0f,  0.05f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

        // Base superior
        -0.025f, 0.02f, -0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.02f, -0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.05f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.025f, 0.02f,  0.05f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Lado frontal (Z+)
        -0.025f, 0.0f,  0.05f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.025f, 0.0f,  0.05f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.05f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.025f, 0.02f,  0.05f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Lado trasero (Z-)
        -0.025f, 0.0f, -0.05f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
         0.025f, 0.0f, -0.05f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.025f, 0.02f, -0.05f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -0.025f, 0.02f, -0.05f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        // Lado derecho (X+)
         0.025f, 0.0f, -0.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.025f, 0.0f,  0.05f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.025f, 0.02f,  0.05f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.025f, 0.02f, -0.05f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

         // Lado izquierdo (X-)
         -0.025f, 0.0f, -0.05f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         -0.025f, 0.0f,  0.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         -0.025f, 0.02f,  0.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         -0.025f, 0.02f, -0.05f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  4, 7, 6,
        8, 9, 10, 10, 11, 8,
        12, 14, 13, 12, 15, 14,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 20, 23, 22
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
// Función para crear una gota de agua (cubo alargado con grosor medio)
SimpleGeometry* CreateGotaAgua(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    // Gotas con grosor medio (0.008) y largas (0.04)
    std::vector<GLfloat> vertices = {
        -0.008f, -0.04f,  0.008f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.008f, -0.04f,  0.008f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.008f,  0.04f,  0.008f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.008f,  0.04f,  0.008f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        -0.008f, -0.04f, -0.008f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -0.008f,  0.04f, -0.008f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.008f,  0.04f, -0.008f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
         0.008f, -0.04f, -0.008f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

         -0.008f,  0.04f, -0.008f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         -0.008f,  0.04f,  0.008f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
          0.008f,  0.04f,  0.008f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
          0.008f,  0.04f, -0.008f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

          -0.008f, -0.04f, -0.008f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
           0.008f, -0.04f, -0.008f,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
           0.008f, -0.04f,  0.008f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          -0.008f, -0.04f,  0.008f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,

           0.008f, -0.04f, -0.008f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
           0.008f,  0.04f, -0.008f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
           0.008f,  0.04f,  0.008f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
           0.008f, -0.04f,  0.008f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

           -0.008f, -0.04f, -0.008f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
           -0.008f, -0.04f,  0.008f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
           -0.008f,  0.04f,  0.008f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
           -0.008f,  0.04f, -0.008f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
// Crear esfera pequeña para partículas de fuegos artificiales
SimpleGeometry* CreateParticulaFuego(GLuint texture) {
    SimpleGeometry* geo = new SimpleGeometry();
    geo->texture = texture;

    std::vector<GLfloat> vertices = {
        // Cara frontal
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Cara trasera
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

         // Cara superior
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

          // Cara inferior
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,

          // Cara derecha
           0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
           0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
           0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

           // Cara izquierda
           -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
           -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    std::vector<GLuint> indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    geo->indexCount = 36;

    glGenVertexArrays(1, &geo->VAO);
    glGenBuffers(1, &geo->VBO);
    glGenBuffers(1, &geo->EBO);

    glBindVertexArray(geo->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    return geo;
}
void CrearExplosion(glm::vec3 posicion, glm::vec3 color) {
    int particulasPorExplosion = 250;  // MUCHAS MÁS PARTÍCULAS
    int creadas = 0;

    for (int i = 0; i < MAX_PARTICULAS && creadas < particulasPorExplosion; i++) {
        if (!particulas[i].activa) {
            particulas[i].activa = true;
            particulas[i].posicion = posicion;
            particulas[i].vida = 3.0f + (rand() % 150) / 50.0f;  // Viven MÁS tiempo
            particulas[i].vidaInicial = particulas[i].vida;
            particulas[i].color = color;
            particulas[i].tamano = 0.35f + (rand() % 150) / 500.0f;  // MUY GRANDES (0.35-0.65)

            // Velocidades MUY RÁPIDAS para explosiones grandes
            float theta = (rand() % 360) * PI / 180.0f;
            float phi = (rand() % 180) * PI / 180.0f;
            float velocidad = 5.0f + (rand() % 600) / 100.0f;  // 5.0 a 11.0 (muy rápido)

            particulas[i].velocidad = glm::vec3(
                velocidad * sin(phi) * cos(theta),
                velocidad * cos(phi),
                velocidad * sin(phi) * sin(theta)
            );

            creadas++;
        }
    }
}
int main()
{
    if (!glfwInit())
    {
        std::cout << "Error al inicializar GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Modelado Jerarquico - Cocina 3D", nullptr, nullptr);

    if (!window)
    {
        std::cout << "Error al crear ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error al inicializar GLEW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int screenWidth = 0, screenHeight = 0;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

    std::cout << "\n=== CONTROLES ===" << std::endl;
    std::cout << "W/A/S/D: Mover cámara" << std::endl;
    std::cout << "Mouse: Mirar alrededor" << std::endl;
    std::cout << "O/C: Abrir/Cerrar puerta del refrigerador" << std::endl;
    std::cout << "N: Mover banderines hacia atrás" << std::endl;
    std::cout << "M: Regresar banderines a posición normal" << std::endl;
    std::cout << "G: Abrir/Cerrar grifo (agua)" << std::endl;
    std::cout << "F: Activar/Desactivar fuegos artificiales" << std::endl;
    std::cout << "1/2/3: Encender/Apagar lámparas de cocina" << std::endl;
    std::cout << "4: Encender/Apagar luz de la Luna" << std::endl;
    std::cout << "ESC: Salir\n" << std::endl;
    // Carga de texturas
    GLuint textureArroz = LoadTexture("images/5138.jpg");
    GLuint textureBarandal = LoadTexture("images/fondo-abstracto-con-textura (1).jpg");
    GLuint textureBase = LoadTexture("images/O5X8JM0.jpg");
    GLuint textureBotes = LoadTexture("images/OE5TN00.jpg");
    GLuint textureBolsaHarinaAbierta1 = LoadTexture("images/Illustration03.jpg");
    GLuint textureBolsaHarinaAbierta = LoadTexture("images/Illustration03.jpg");
    GLuint textureBolsasHarina = LoadTexture("images/Illustration03.jpg");
    GLuint textureBoteBasura = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureCadenaLinterna = LoadTexture("images/5178.jpg");
    GLuint textureCaja = LoadTexture("images/16.jpg");
    GLuint textureCajas2 = LoadTexture("images/16.jpg");
    GLuint textureCajasMadera = LoadTexture("images/370.jpg");
    GLuint textureCampana = LoadTexture("images/5178.jpg");
    GLuint textureCortina2 = LoadTexture("images/preview.jpg");
    GLuint textureCortina = LoadTexture("images/169180-OVUV5U-320.jpg");
    GLuint textureCortinas = LoadTexture("images/preview.jpg");
    GLuint textureCubeta = LoadTexture("images/OR6ILJ0.jpg");
    GLuint textureElectricidad = LoadTexture("images/5178.jpg");
    GLuint textureEscaleras = LoadTexture("images/3690.jpg");
    GLuint textureEstufa = LoadTexture("images/textura-de-placa-de-metal.jpg");
    GLuint textureFregadero = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureGas = LoadTexture("images/OQV54F0.jpg");
    GLuint textureJalador = LoadTexture("images/OR6ILJ0.jpg");
    GLuint textureLamparasCocina = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureLamparasCocina1 = LoadTexture("images/2259.jpg");
    GLuint textureLamparas1 = LoadTexture("images/2259.jpg");
    GLuint textureLamparas = LoadTexture("images/textura-de-placa-de-metal.jpg");
    GLuint textureLibrero = LoadTexture("images/3690.jpg");
    GLuint textureLuna = LoadTexture("images/2259.jpg");
    GLuint textureLinternaLocal = LoadTexture("images/2259.jpg");
    GLuint textureMesa = LoadTexture("images/OQV54F0.jpg");
    GLuint textureOlla = LoadTexture("images/5178.jpg");
    GLuint textureOlla1 = LoadTexture("images/5178.jpg");
    GLuint textureOlla2 = LoadTexture("images/acuarela-de-oro-liquido-espacio-de-copia.jpg");
    GLuint textureOlla3 = LoadTexture("images/acuarela-de-oro-liquido-espacio-de-copia.jpg");
    GLuint texturePasto = LoadTexture("images/preview1.jpg");
    GLuint texturePared3 = LoadTexture("images/3690.jpg");
    GLuint texturePared4 = LoadTexture("images/3690.jpg");
    GLuint texturePared = LoadTexture("images/18366.jpg");
    GLuint texturePlatos = LoadTexture("images/OIVY360.jpg");
    GLuint texturePiso2 = LoadTexture("images/8469.jpg");
    GLuint texturePrimerPiso = LoadTexture("images/liquid-marbling-paint-texture-background-fluid-painting-abstract-texture-intensive-color-mix-wallpaper.jpg");
    GLuint texturePuertas = LoadTexture("images/3690.jpg");
    GLuint textureRefrigerador = LoadTexture("images/OIVY360.jpg");
    GLuint textureRefrigerador1 = LoadTexture("images/OIVY360.jpg");
    GLuint textureSilla2 = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureSilla = LoadTexture("images/textura-grunge-roja.jpg");
    GLuint textureTapa = LoadTexture("images/OR6ILJ0.jpg");
    GLuint textureTapa1 = LoadTexture("images/OR6ILJ0.jpg");
    GLuint textureTapa2 = LoadTexture("images/OR6ILJ0.jpg");
    GLuint textureTanqueAgua = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureTapeteAlmacen = LoadTexture("images/47074.jpg");
    GLuint textureTapeteCocina = LoadTexture("images/abstract-blur-empty-green-gradient-studio-well-use-as-background-website-template-frame-business-report.jpg");
    GLuint textureTazon = LoadTexture("images/OIVY360.jpg");
    GLuint textureTazones2 = LoadTexture("images/OIVY360.jpg");
    GLuint textureTejado = LoadTexture("images/6593778.jpg");
    GLuint textureTejadoMini = LoadTexture("images/3690.jpg");
    GLuint textureTejadoMiniPiso2 = LoadTexture("images/3690.jpg");
    GLuint textureTuboFuera = LoadTexture("images/5178.jpg");
    GLuint textureTuboGas = LoadTexture("images/5178.jpg");
    GLuint textureVentanaSegundoPiso = LoadTexture("images/3690.jpg");
    GLuint textureBanderines = LoadTexture("images/preview.jpg");

    // Carga de modelos
    Model Arroz((char*)"Models/Arroz.obj");
    Model Barandal((char*)"Models/Barandal.obj");
    Model Base((char*)"Models/Base.obj");
    Model Botes((char*)"Models/Botes.obj");
    Model BolsaHarinaAbierta1((char*)"Models/BolsaHarinaAbierta1.obj");
    Model BolsaHarinaAbierta((char*)"Models/BolsaHarinaAbierta.obj");
    Model BolsasHarina((char*)"Models/BolsasHarina.obj");
    Model BoteBasura((char*)"Models/BoteBasura.obj");
    Model CadenaLinterna((char*)"Models/CadenaLinterna.obj");
    Model Caja((char*)"Models/Caja.obj");
    Model Cajas2((char*)"Models/Cajas2.obj");
    Model CajasMadera((char*)"Models/CajasMadera.obj");
    Model Campana((char*)"Models/Campana.obj");
    Model Cortina2((char*)"Models/Cortina2.obj");
    Model Cortina((char*)"Models/Cortina.obj");
    Model Cortinas((char*)"Models/Cortinas.obj");
    Model Cubeta((char*)"Models/Cubeta.obj");
    Model Electricidad((char*)"Models/Electricidad.obj");
    Model Escaleras((char*)"Models/Escaleras.obj");
    Model Estufa((char*)"Models/Estufa.obj");
    Model Fregadero((char*)"Models/Fregadero.obj");
    Model Gas((char*)"Models/Gas.obj");
    Model Jalador((char*)"Models/Jalador.obj");
    Model LamparasCocina((char*)"Models/LamparasCocina.obj");
    Model LamparasCocina1((char*)"Models/LamparasCocina1.obj");
    Model Librero((char*)"Models/Librero.obj");
    Model Luna((char*)"Models/Luna.obj");
    Model LinternaLocal((char*)"Models/LinternaLocal.obj");
    Model Mesa((char*)"Models/Mesa.obj");
    Model Olla((char*)"Models/Olla.obj");
    Model Olla1((char*)"Models/Olla1.obj");
    Model Olla2((char*)"Models/Olla2.obj");
    Model Olla3((char*)"Models/Olla3.obj");
    Model Pasto((char*)"Models/Pasto.obj");
    Model Pared3((char*)"Models/Pared3.obj");
    Model Pared4((char*)"Models/Pared4.obj");
    Model Pared((char*)"Models/Pared.obj");
    Model Platos((char*)"Models/Platos.obj");
    Model Piso2((char*)"Models/Piso2.obj");
    Model PrimerPiso((char*)"Models/PrimerPiso.obj");
    Model Puertas((char*)"Models/Puertas.obj");
    Model Refrigerador((char*)"Models/Refrigerador.obj");
    Model Refrigerador1((char*)"Models/Refrigerador1.obj");
    Model Silla2((char*)"Models/Silla2.obj");
    Model Silla((char*)"Models/Silla.obj");
    Model Tapa((char*)"Models/Tapa.obj");
    Model Tapa1((char*)"Models/Tapa1.obj");
    Model Tapa2((char*)"Models/Tapa2.obj");
    Model TanqueAgua((char*)"Models/TanqueAgua.obj");
    Model TapeteAlmacen((char*)"Models/TapeteAlmacen.obj");
    Model TapeteCocina((char*)"Models/TapeteCocina.obj");
    Model Tazon((char*)"Models/Tazon.obj");
    Model Tazones2((char*)"Models/Tazones2.obj");
    Model Tejado((char*)"Models/Tejado.obj");
    Model TejadoMini((char*)"Models/TejadoMini.obj");
    Model TejadoMiniPiso2((char*)"Models/TejadoMiniPiso2.obj");
    Model TuboFuera((char*)"Models/TuboFuera.obj");
    Model TuboGas((char*)"Models/TuboGas.obj");
    Model VentanaSegundoPiso((char*)"Models/VentanaSegundoPiso.obj");
    Model Banderines((char*)"Models/Banderines.obj");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

    // Crear geometrías personalizadas
    GLuint textureReloj = LoadTexture("images/OIVY360.jpg");
    GLuint textureManecilla = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureCubo = LoadTexture("images/16.jpg");
    GLuint textureMesaCocina = LoadTexture("images/3690.jpg");
    GLuint textureNarutomaki = LoadTexture("images/OIVY360.jpg");
    GLuint textureSarten = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureTabla = LoadTexture("images/fondo-de-diseno-de-textura-de-madera-de-roble.jpg");
    GLuint textureCuchilloHoja = LoadTexture("images/fondo-abstracto-con-textura.jpg");
    GLuint textureCuchilloMango = LoadTexture("images/3690.jpg");
    GLuint textureAgua = LoadTexture("images/papel-pintado-azul-monocromatico-minimalista.jpg");
    GLuint textureParticula = LoadTexture("images/acuarela-de-oro-liquido-espacio-de-copia.jpg"); // Textura brillante

    SimpleGeometry* ParticulaFuego = CreateParticulaFuego(textureParticula);
    SimpleGeometry* RelojCirculo = CreateCircle(textureReloj, 0.5f, 60);
    SimpleGeometry* ManecillaHora = CreateClockHand(textureManecilla, 0.05f, 0.3f);
    SimpleGeometry* ManecillaMinuto = CreateClockHand(textureManecilla, 0.03f, 0.45f);
    SimpleGeometry* Cubo = CreateCube(textureCubo);
    SimpleGeometry* MesaTop = CreateMesaTop(textureMesaCocina);
    SimpleGeometry* Pata = CreatePataMesa(textureMesaCocina);
    SimpleGeometry* Narutomaki = CreateNarutomaki(textureNarutomaki);
    SimpleGeometry* Sarten = CreateSarten(textureSarten);
    SimpleGeometry* TablaPicar = CreateTablaPicar(textureTabla);
    SimpleGeometry* HojaCuchillo = CreateHojaCuchillo(textureCuchilloHoja);
    SimpleGeometry* MangoCuchillo = CreateMangoCuchillo(textureCuchilloMango);
    SimpleGeometry* GotaAgua = CreateGotaAgua(textureAgua);

    // Inicializar el vector de gotas
    gotas.resize(MAX_GOTAS);
    for (int i = 0; i < MAX_GOTAS; i++) {
        gotas[i].activa = false;
        gotas[i].velocidad = 2.0f;
    }
    // Inicializar partículas de fuegos artificiales
    particulas.resize(MAX_PARTICULAS);
    for (int i = 0; i < MAX_PARTICULAS; i++) {
        particulas[i].activa = false;
    }
    srand(time(NULL)); // Para números aleatorios
    glm::vec3 posReloj = glm::vec3(-2.0f, 2.0f, 0.15f);
    glm::vec3 posCaja = glm::vec3(1.50f, 4.80f, -0.30f);
    glm::vec3 posMesa = glm::vec3(0.80f, 5.0f, -2.40f);
    glm::vec3 posTabla = glm::vec3(-1.50f, 1.17f, 1.5f);
    glm::vec3 posSarten = glm::vec3(0.80f, 5.05f, -2.40f);

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        ProcessMovement();

        glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glm::vec3 cameraPos = camera.GetPosition();
        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f, 0.05f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.1f, 0.1f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.3f);

        for (int i = 0; i < 4; ++i) {
            std::string prefix = "pointLights[" + std::to_string(i) + "]";

            glm::vec3 ambient, diffuse, specular;
            float constant, linear, quadratic;

            if (i == 3) {
                ambient = lightStates[i] ? glm::vec3(0.2f, 0.2f, 0.3f) : glm::vec3(0.0f);
                diffuse = lightStates[i] ? glm::vec3(0.8f, 0.8f, 1.2f) : glm::vec3(0.0f);
                specular = lightStates[i] ? glm::vec3(1.0f, 1.0f, 1.3f) : glm::vec3(0.0f);
                constant = 1.0f;
                linear = 0.014f;
                quadratic = 0.0007f;
            }
            else {
                ambient = lightStates[i] ? glm::vec3(0.15f, 0.05f, 0.0f) : glm::vec3(0.0f);       // tenue
                diffuse = lightStates[i] ? glm::vec3(1.0f, 0.4f, 0.1f) : glm::vec3(0.0f);         // naranja-rojizo “chakra”
                specular = lightStates[i] ? glm::vec3(1.0f, 0.6f, 0.2f) : glm::vec3(0.0f);        // brillo cálido

                // Atenuación suave para que no se expanda demasiado
                constant = 1.0f;
                linear = 0.07f;
                quadratic = 0.017f;
            }

            glUniform3f(glGetUniformLocation(lightingShader.Program, (prefix + ".position").c_str()),
                pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (prefix + ".ambient").c_str()),
                ambient.x, ambient.y, ambient.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (prefix + ".diffuse").c_str()),
                diffuse.x, diffuse.y, diffuse.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (prefix + ".specular").c_str()),
                specular.x, specular.y, specular.z);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (prefix + ".constant").c_str()), constant);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (prefix + ".linear").c_str()), linear);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (prefix + ".quadratic").c_str()), quadratic);
        }

        // Inicializar spotLight (apagado) - IMPORTANTE para que el shader funcione
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), 0.0f, -1.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1);

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

        // IMPORTANTE: Configurar los samplers de textura
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 0);
       
        // ===== PRIMERO: FUEGOS ARTIFICIALES (para que estén detrás) =====
        if (fuegosActivos) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // NO desactives el depth test aquí para que respeten la profundidad
            // glDisable(GL_DEPTH_TEST); <-- QUITA ESTA LÍNEA

            for (int i = 0; i < MAX_PARTICULAS; i++) {
                if (particulas[i].activa) {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, particulas[i].posicion);

                    float factorVida = particulas[i].vida / particulas[i].vidaInicial;
                    float escala = particulas[i].tamano * factorVida;
                    model = glm::scale(model, glm::vec3(escala));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glBindTexture(GL_TEXTURE_2D, textureParticula);
                    glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 128.0f);

                    ParticulaFuego->Draw();
                }
            }

            // glEnable(GL_DEPTH_TEST); <-- QUITA ESTA LÍNEA TAMBIÉN
            glDisable(GL_BLEND);
        }
        // Renderizado de objetos opacos
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Bind texture y dibujar Arroz
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureArroz);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 48.0f);
        Arroz.Draw(lightingShader);

        // Base
        glBindTexture(GL_TEXTURE_2D, textureBase);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
        Base.Draw(lightingShader);

        // Barandal
        glBindTexture(GL_TEXTURE_2D, textureBarandal);
        Barandal.Draw(lightingShader);

        // Botes
        glBindTexture(GL_TEXTURE_2D, textureBotes);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 24.0f);
        Botes.Draw(lightingShader);

        // BolsaHarinaAbierta
        glBindTexture(GL_TEXTURE_2D, textureBolsaHarinaAbierta);
        BolsaHarinaAbierta.Draw(lightingShader);

        // BolsaHarinaAbierta1
        glBindTexture(GL_TEXTURE_2D, textureBolsaHarinaAbierta1);
        BolsaHarinaAbierta1.Draw(lightingShader);

        // BolsasHarina
        glBindTexture(GL_TEXTURE_2D, textureBolsasHarina);
        BolsasHarina.Draw(lightingShader);

        // BoteBasura
        glBindTexture(GL_TEXTURE_2D, textureBoteBasura);
        BoteBasura.Draw(lightingShader);

        // CadenaLinterna
        glBindTexture(GL_TEXTURE_2D, textureCadenaLinterna);
        CadenaLinterna.Draw(lightingShader);

        // Caja
        glBindTexture(GL_TEXTURE_2D, textureCaja);
        Caja.Draw(lightingShader);

        // Cajas2
        glBindTexture(GL_TEXTURE_2D, textureCajas2);
        Cajas2.Draw(lightingShader);

        // CajasMadera
        glBindTexture(GL_TEXTURE_2D, textureCajasMadera);
        CajasMadera.Draw(lightingShader);

        // Campana
        glBindTexture(GL_TEXTURE_2D, textureCampana);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 56.0f);
        Campana.Draw(lightingShader);

        // Cortina
        glBindTexture(GL_TEXTURE_2D, textureCortina);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        Cortina.Draw(lightingShader);

        // Cortinas
        glBindTexture(GL_TEXTURE_2D, textureCortinas);
        Cortinas.Draw(lightingShader);

        // Cortina2
        glBindTexture(GL_TEXTURE_2D, textureCortina2);
        Cortina2.Draw(lightingShader);

        // Cubeta
        glBindTexture(GL_TEXTURE_2D, textureCubeta);
        Cubeta.Draw(lightingShader);

        // Electricidad
        glBindTexture(GL_TEXTURE_2D, textureElectricidad);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 40.0f);
        Electricidad.Draw(lightingShader);

        // Escaleras
        glBindTexture(GL_TEXTURE_2D, textureEscaleras);
        Escaleras.Draw(lightingShader);

        // Estufa
        glBindTexture(GL_TEXTURE_2D, textureEstufa);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 64.0f);
        Estufa.Draw(lightingShader);

        // Fregadero
        glBindTexture(GL_TEXTURE_2D, textureFregadero);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 72.0f);
        Fregadero.Draw(lightingShader);

        // Gas
        glBindTexture(GL_TEXTURE_2D, textureGas);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 48.0f);
        Gas.Draw(lightingShader);

        // Jalador
        glBindTexture(GL_TEXTURE_2D, textureJalador);
        Jalador.Draw(lightingShader);


        // LamparasCocina
        glBindTexture(GL_TEXTURE_2D, textureLamparasCocina);
        LamparasCocina.Draw(lightingShader);

        // Librero
        glBindTexture(GL_TEXTURE_2D, textureLibrero);
        Librero.Draw(lightingShader);

        // Mesa
        glBindTexture(GL_TEXTURE_2D, textureMesa);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 40.0f);
        Mesa.Draw(lightingShader);

        // Olla
        glBindTexture(GL_TEXTURE_2D, textureOlla);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 96.0f);
        Olla.Draw(lightingShader);

        // Olla1
        glBindTexture(GL_TEXTURE_2D, textureOlla1);
        Olla1.Draw(lightingShader);

        // Olla2
        glBindTexture(GL_TEXTURE_2D, textureOlla2);
        Olla2.Draw(lightingShader);

        // Olla3
        glBindTexture(GL_TEXTURE_2D, textureOlla3);
        Olla3.Draw(lightingShader);

        // Pasto
        glBindTexture(GL_TEXTURE_2D, texturePasto);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);
        Pasto.Draw(lightingShader);

        // Pared3
        glBindTexture(GL_TEXTURE_2D, texturePared3);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 24.0f);
        Pared3.Draw(lightingShader);

        // Pared4
        glBindTexture(GL_TEXTURE_2D, texturePared4);
        Pared4.Draw(lightingShader);

        // Pared
        glBindTexture(GL_TEXTURE_2D, texturePared);
        Pared.Draw(lightingShader);

        // Platos
        glBindTexture(GL_TEXTURE_2D, texturePlatos);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 56.0f);
        Platos.Draw(lightingShader);

        // PrimerPiso
        glBindTexture(GL_TEXTURE_2D, texturePrimerPiso);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
        PrimerPiso.Draw(lightingShader);

        // Piso2
        glBindTexture(GL_TEXTURE_2D, texturePiso2);
        Piso2.Draw(lightingShader);

        // Puertas
        glBindTexture(GL_TEXTURE_2D, texturePuertas);
        Puertas.Draw(lightingShader);

        // Refrigerador
        glBindTexture(GL_TEXTURE_2D, textureRefrigerador);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 80.0f);
        Refrigerador.Draw(lightingShader);

        // Silla2
        glBindTexture(GL_TEXTURE_2D, textureSilla2);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 20.0f);
        Silla2.Draw(lightingShader);

        // Silla
        glBindTexture(GL_TEXTURE_2D, textureSilla);
        Silla.Draw(lightingShader);

        // Tapa
        glBindTexture(GL_TEXTURE_2D, textureTapa);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 48.0f);
        Tapa.Draw(lightingShader);

        // Tapa1
        glBindTexture(GL_TEXTURE_2D, textureTapa1);
        Tapa1.Draw(lightingShader);

        // Tapa2
        glBindTexture(GL_TEXTURE_2D, textureTapa2);
        Tapa2.Draw(lightingShader);

        // TapeteAlmacen
        glBindTexture(GL_TEXTURE_2D, textureTapeteAlmacen);
        TapeteAlmacen.Draw(lightingShader);

        // TanqueAgua
        glBindTexture(GL_TEXTURE_2D, textureTanqueAgua);
        TanqueAgua.Draw(lightingShader);

        // TapeteCocina
        glBindTexture(GL_TEXTURE_2D, textureTapeteCocina);
        TapeteCocina.Draw(lightingShader);

        // Tazon
        glBindTexture(GL_TEXTURE_2D, textureTazon);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 64.0f);
        Tazon.Draw(lightingShader);

        // Tazones2
        glBindTexture(GL_TEXTURE_2D, textureTazones2);
        Tazones2.Draw(lightingShader);

        // Tejado
        glBindTexture(GL_TEXTURE_2D, textureTejado);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        Tejado.Draw(lightingShader);

        // TejadoMini
        glBindTexture(GL_TEXTURE_2D, textureTejadoMini);
        TejadoMini.Draw(lightingShader);

        // TejadoMiniPiso2
        glBindTexture(GL_TEXTURE_2D, textureTejadoMiniPiso2);
        TejadoMiniPiso2.Draw(lightingShader);

        // TuboFuera
        glBindTexture(GL_TEXTURE_2D, textureTuboFuera);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 80.0f);
        TuboFuera.Draw(lightingShader);

        // TuboGas
        glBindTexture(GL_TEXTURE_2D, textureTuboGas);
        TuboGas.Draw(lightingShader);

        // VentanaSegundoPiso
        glBindTexture(GL_TEXTURE_2D, textureVentanaSegundoPiso);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 24.0f);
        VentanaSegundoPiso.Draw(lightingShader);
    
        // ===== BANDERINES CON CONTROL MANUAL (ADELANTE/ATRÁS) =====
        model = glm::mat4(1.0f);

        // La rotación se aplica en el eje X para mover la punta hacia adelante/atrás
        model = glm::rotate(model, glm::radians(banderinesInclinacion), glm::vec3(1.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindTexture(GL_TEXTURE_2D, textureBanderines);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        Banderines.Draw(lightingShader);

        // ===== RELOJ =====
        model = glm::mat4(1.0f);
        model = glm::translate(model, posReloj);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
        RelojCirculo->Draw();

        // Manecilla de hora (gira lento)
        float horaAngulo = (float)glfwGetTime() * 0.1f;
        model = glm::translate(glm::mat4(1.0f), posReloj);
        model = glm::rotate(model, horaAngulo, glm::vec3(0.0f, 0.0f, -1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ManecillaHora->Draw();

        // Manecilla de minuto (gira rápido)
        float minutoAngulo = (float)glfwGetTime() * 0.5f;
        model = glm::translate(glm::mat4(1.0f), posReloj);
        model = glm::rotate(model, minutoAngulo, glm::vec3(0.0f, 0.0f, -1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        ManecillaMinuto->Draw();

        // ===== CUBO (CAJA) =====
        model = glm::mat4(1.0f);
        model = glm::translate(model, posCaja);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        Cubo->Draw();

        // ===== MESA CON PATAS =====
        // Superficie de la mesa
        model = glm::mat4(1.0f);
        model = glm::translate(model, posMesa);
        model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 40.0f);
        MesaTop->Draw();

        // Pata 1 (esquina -X, -Z)
        model = glm::mat4(1.0f);
        model = glm::translate(model, posMesa + glm::vec3(-0.65f, 0.0f, -0.65f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Pata->Draw();

        // Pata 2 (esquina +X, -Z)
        model = glm::mat4(1.0f);
        model = glm::translate(model, posMesa + glm::vec3(0.65f, 0.0f, -0.65f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Pata->Draw();

        // Pata 3 (esquina -X, +Z)
        model = glm::mat4(1.0f);
        model = glm::translate(model, posMesa + glm::vec3(-0.65f, 0.0f, 0.65f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Pata->Draw();

        // Pata 4 (esquina +X, +Z)
        model = glm::mat4(1.0f);
        model = glm::translate(model, posMesa + glm::vec3(0.65f, 0.0f, 0.65f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Pata->Draw();

        // ===== TABLA DE PICAR =====
        model = glm::mat4(1.0f);
        model = glm::translate(model, posTabla);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 24.0f);
        TablaPicar->Draw();

        // ===== SARTÉN =====
        model = glm::mat4(1.0f);
        model = glm::translate(model, posSarten);
        model = glm::scale(model, glm::vec3(2.8f, 1.0f, 2.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 64.0f);
        Sarten->Draw();

        // ===== NARUTOMAKI (3 piezas en fila) =====
        for (int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.60f + i * 0.1f, 1.20f, 1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 20.0f);
            Narutomaki->Draw();
        }
        // ===== CUCHILLO (HOJA + MANGO) =====
        glm::vec3 posCuchillo = glm::vec3(-1.80f, 1.20f, 1.5f);

        // Hoja del cuchillo (rotada 45 grados)
        model = glm::mat4(1.0f);
        model = glm::translate(model, posCuchillo);
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 96.0f);
        HojaCuchillo->Draw();

        // Mango del cuchillo (atrás de la hoja)
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.25f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        MangoCuchillo->Draw();

        // ===== GOTAS DE AGUA =====
        if (aguaCayendo) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            for (int i = 0; i < MAX_GOTAS; i++) {
                if (gotas[i].activa) {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, gotas[i].posicion);
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glBindTexture(GL_TEXTURE_2D, textureAgua);
                    glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 128.0f);
                    GotaAgua->Draw();
                }
            }

            glDisable(GL_BLEND);
        }

        // Animación del refrigerador
        glm::vec3 posBisagraRefrigerador = glm::vec3(2.5f, 1.0f, -3.60f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, posBisagraRefrigerador);
        model = glm::rotate(model, glm::radians(-puertaRefrigerador), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, -posBisagraRefrigerador);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindTexture(GL_TEXTURE_2D, textureRefrigerador1);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 80.0f);
        Refrigerador1.Draw(lightingShader);


        // Objetos transparentes (lámparas)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);

        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 128.0f);

        // LamparasCocina1
        glBindTexture(GL_TEXTURE_2D, textureLamparasCocina1);
        LamparasCocina1.Draw(lightingShader);

        // LinternaLocal
        glBindTexture(GL_TEXTURE_2D, textureLinternaLocal);
        LinternaLocal.Draw(lightingShader);

        // Luna
        glBindTexture(GL_TEXTURE_2D, textureLuna);
        Luna.Draw(lightingShader);

        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
    }
    delete RelojCirculo;
    delete ManecillaHora;
    delete ManecillaMinuto;
    delete Cubo;
    delete MesaTop;
    delete Pata;
    delete Narutomaki;
    delete Sarten;
    delete TablaPicar;
    delete HojaCuchillo;
    delete MangoCuchillo;
    delete GotaAgua;
    delete ParticulaFuego;

    glfwTerminate();
    return EXIT_SUCCESS;
}

void ProcessMovement()
{
    // Movimiento de cámara
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Animación automática de la puerta
    if (animandoPuerta) {
        if (abs(puertaRefrigerador - puertaDestino) > 0.1f) {
            if (puertaRefrigerador < puertaDestino)
                puertaRefrigerador += 80.0f * deltaTime;
            else
                puertaRefrigerador -= 80.0f * deltaTime;
        }
        else {
            puertaRefrigerador = puertaDestino;
            animandoPuerta = false;
        }
    }

    // **ANIMACIÓN: Banderines moviéndose hacia adelante/atrás**
    if (animandoBanderines) {
        if (abs(banderinesInclinacion - banderinesDestino) > 0.1f) {
            if (banderinesInclinacion < banderinesDestino)
                banderinesInclinacion += 60.0f * deltaTime;
            else
                banderinesInclinacion -= 60.0f * deltaTime;
        }
        else {
            banderinesInclinacion = banderinesDestino;
            animandoBanderines = false;
        }
    }

    // **ANIMACIÓN: Agua cayendo del grifo**
    if (aguaCayendo) {
        // Crear nuevas gotas
        if (currentFrame - tiempoUltimaGota >= intervaloGotas) {
            for (int i = 0; i < MAX_GOTAS; i++) {
                if (!gotas[i].activa) {
                    gotas[i].activa = true;
                    gotas[i].posicion = posicionGrifo;
                    gotas[i].velocidad = 2.0f + (rand() % 100) / 100.0f;
                    tiempoUltimaGota = currentFrame;
                    break;
                }
            }
        }

        // Actualizar posición de gotas activas
        for (int i = 0; i < MAX_GOTAS; i++) {
            if (gotas[i].activa) {
                gotas[i].posicion.y -= gotas[i].velocidad * deltaTime;
                gotas[i].velocidad += 3.0f * deltaTime; // Gravedad

                // Desactivar gota si llega al fregadero
                if (gotas[i].posicion.y < 1.0f) {
                    gotas[i].activa = false;
                }
            }
        }
    }
    // **ANIMACIÓN: Fuegos artificiales**
    if (fuegosActivos) {
        if (currentFrame - tiempoUltimoFuego >= intervaloFuegos) {
            // MÁS SEPARADAS HORIZONTALMENTE (de -8 a +8)
            glm::vec3 posExplosion = glm::vec3(
                -8.0f + (rand() % 1600) / 100.0f,   // -8 a 8 en X (MUY ANCHO)
                7.0f + (rand() % 200) / 100.0f,     // 7 a 9 en Y (altura similar)
                -10.0f                               // Z FIJO (sin profundidad)
            );

            // Colores vivos y variados
            int tipoColor = rand() % 5;
            glm::vec3 colorExplosion;

            switch (tipoColor) {
            case 0: colorExplosion = glm::vec3(1.0f, 0.2f, 0.2f); break; // Rojo
            case 1: colorExplosion = glm::vec3(0.2f, 1.0f, 0.2f); break; // Verde
            case 2: colorExplosion = glm::vec3(0.2f, 0.2f, 1.0f); break; // Azul
            case 3: colorExplosion = glm::vec3(1.0f, 1.0f, 0.2f); break; // Amarillo
            case 4: colorExplosion = glm::vec3(1.0f, 0.2f, 1.0f); break; // Magenta
            }

            CrearExplosion(posExplosion, colorExplosion);
            tiempoUltimoFuego = currentFrame;

            std::cout << "¡BOOM! Explosión en el fondo Z=" << posExplosion.z << std::endl;
        }

        // Actualizar partículas activas
        for (int i = 0; i < MAX_PARTICULAS; i++) {
            if (particulas[i].activa) {
                // Actualizar posición
                particulas[i].posicion += particulas[i].velocidad * deltaTime;

                // Aplicar gravedad suave
                particulas[i].velocidad.y -= 2.5f * deltaTime;

                // Fricción del aire
                particulas[i].velocidad *= 0.98f;

                // Reducir vida
                particulas[i].vida -= deltaTime;

                // Desactivar si ya no tiene vida
                if (particulas[i].vida <= 0.0f) {
                    particulas[i].activa = false;
                }
            }
        }
    }
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    (void)scancode;
    (void)mode;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_1:
            lightStates[0] = !lightStates[0];
            std::cout << "Luz 1 (LamparasCocina1): " << (lightStates[0] ? "ENCENDIDA" : "APAGADA") << std::endl;
            break;
        case GLFW_KEY_2:
            lightStates[1] = !lightStates[1];
            std::cout << "Luz 2 (Lamparas1): " << (lightStates[1] ? "ENCENDIDA" : "APAGADA") << std::endl;
            break;
        case GLFW_KEY_3:
            lightStates[2] = !lightStates[2];
            std::cout << "Luz 3 (LinternaLocal): " << (lightStates[2] ? "ENCENDIDA" : "APAGADA") << std::endl;
            break;
        case GLFW_KEY_4:
            lightStates[3] = !lightStates[3];
            std::cout << "Luz Luna: " << (lightStates[3] ? "ENCENDIDA" : "APAGADA") << std::endl;
            break;
        case GLFW_KEY_O:
            puertaDestino = 90.0f;  // Abrir
            animandoPuerta = true;
            std::cout << "Abriendo puerta del refrigerador..." << std::endl;
            break;
        case GLFW_KEY_C:
            puertaDestino = 0.0f;   // Cerrar
            animandoPuerta = true;
            std::cout << "Cerrando puerta del refrigerador..." << std::endl;
            break;
        case GLFW_KEY_N: // Mover banderines hacia ATRÁS
            banderinesDestino = -banderinesMaxInclinacion;
            animandoBanderines = true;
            std::cout << "Banderines moviéndose hacia atrás..." << std::endl;
            break;

        case GLFW_KEY_M: // Regresar banderines a posición NORMAL
            banderinesDestino = 0.0f;
            animandoBanderines = true;
            std::cout << "Banderines regresando a posición normal..." << std::endl;
            break;
        case GLFW_KEY_G: // Toggle agua del grifo
            aguaCayendo = !aguaCayendo;
            std::cout << "Agua del grifo: " << (aguaCayendo ? "ABIERTA" : "CERRADA") << std::endl;
            if (!aguaCayendo) {
                for (int i = 0; i < MAX_GOTAS; i++) {
                    gotas[i].activa = false;
                }
            }
            break;

        case GLFW_KEY_F: // Toggle fuegos artificiales
            fuegosActivos = !fuegosActivos;
            std::cout << "Fuegos artificiales: " << (fuegosActivos ? "ACTIVADOS" : "DESACTIVADOS") << std::endl;
            if (!fuegosActivos) {
                for (int i = 0; i < MAX_PARTICULAS; i++) {
                    particulas[i].activa = false;
                }
            }
            break;
        }
    }
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;

    if (firstMouse)
    {
        lastX = (GLfloat)xpos;
        lastY = (GLfloat)ypos;
        firstMouse = false;
    }

    GLfloat xOffset = (GLfloat)(xpos - lastX);
    GLfloat yOffset = (GLfloat)(lastY - ypos);

    lastX = (GLfloat)xpos;
    lastY = (GLfloat)ypos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}