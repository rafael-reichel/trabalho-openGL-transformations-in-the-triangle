#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Shaders
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = aColor;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)glsl";

// Função para compilar shaders
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Verificar erros de compilação
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERRO::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "::COMPILACAO_FALHOU\n" << infoLog << std::endl;
    }
    return shader;
}

// Função para criar programa de shader
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Verificar erros de linking
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERRO::PROGRAMA::SHADER::LINKING_FALHOU\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    // Configurar GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criar janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangulo OpenGL com Transformacoes (Lento)", NULL, NULL);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Carregar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Criar e compilar shaders
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Dados do triângulo (posições e cores)
    float vertices[] = {
        // Posições         // Cores
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Vértice inferior esquerdo (vermelho)
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Vértice inferior direito (verde)
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // Vértice superior (azul)
    };

    // Configurar VAO e VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo de posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo de cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {
        // Limpar buffer de cor
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Usar o programa de shader
        glUseProgram(shaderProgram);

        // Criar matrizes de transformação
        float time = glfwGetTime();

        // REDUZIR VELOCIDADE - dividir o tempo por um fator
        float slowTime = time / 2.0f; // Metade da velocidade original

        // Matriz de translação (mais lenta)
        float transX = sin(slowTime) * 0.3f;  // Amplitude reduzida
        float transY = cos(slowTime) * 0.3f;  // Amplitude reduzida

        // Matriz de rotação (mais lenta)
        float angle = slowTime * 25.0f; // Rotação de 25 graus por segundo (era 50)

        // Matriz de escala (pulsação mais lenta)
        float scale = 0.7f + sin(slowTime) * 0.1f; // Variação menor e mais lenta

        // Construir matriz de transformação combinada
        float transform[16] = {
            scale * cos(angle), -scale * sin(angle), 0.0f, transX,
            scale * sin(angle),  scale * cos(angle), 0.0f, transY,
            0.0f,                0.0f,               1.0f, 0.0f,
            0.0f,                0.0f,               0.0f, 1.0f
        };

        // Passar a matriz de transformação para o shader
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        // Desenhar o triângulo
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Trocar buffers e verificar eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpar recursos
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
