#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "tiny_obj_loader.h"

using namespace std;

GLuint vao, vbo[4], program;
glm::mat4 model, mvp;
int indicesCount=0;

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void setup_shader(const char *vertex_shader, const char *fragment_shader)
{
    GLuint vs=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, (const GLchar**)&vertex_shader, nullptr);

    glCompileShader(vs);

    int status, maxLength;
    char *infoLog=nullptr;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

        /* The maxLength includes the NULL character */
        infoLog = new char[maxLength];

        glGetShaderInfoLog(vs, maxLength, &maxLength, infoLog);

        fprintf(stderr, "Vertex Shader Error: %s\n", infoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        /* In this simple program, we'll just leave */
        delete [] infoLog;
        return;
    }

    GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, (const GLchar**)&fragment_shader, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status==GL_FALSE)
    {
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

        /* The maxLength includes the NULL character */
        infoLog = new char[maxLength];

        glGetShaderInfoLog(fs, maxLength, &maxLength, infoLog);

        fprintf(stderr, "Fragment Shader Error: %s\n", infoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        /* In this simple program, we'll just leave */
        delete [] infoLog;
        return;
    }

    program=glCreateProgram();
    // Attach our shaders to our program
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(status==GL_FALSE)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        

        /* The maxLength includes the NULL character */
        infoLog = new char[maxLength];
        glGetProgramInfoLog(program, maxLength, NULL, infoLog);

        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

        fprintf(stderr, "Link Error: %s\n", infoLog);

        /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
        /* In this simple program, we'll just leave */
        delete [] infoLog;
        return;
    }
}

static string readfile(const char *filename)
{
    std::ifstream ifs(filename);
    if(!ifs)
        exit(EXIT_FAILURE);
    return std::string( (std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
}


static void setup(const char *filename)
{
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;

    string err = tinyobj::LoadObj(shapes, materials, filename);

    if (!err.empty()||shapes.size()==0)
    {
        cerr<<err<<endl;
        exit(1);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(4, vbo);

    glBindVertexArray(vao);

    // Upload postion array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.positions.size(),
        shapes[0].mesh.positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if(shapes[0].mesh.texcoords.size()>0)
    {

        // Upload texCoord array
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.texcoords.size(),
            shapes[0].mesh.texcoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if(shapes[0].mesh.normals.size()>0)
    {
        // Upload normal array
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.normals.size(),
            shapes[0].mesh.normals.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // Setup index buffer for glDrawElements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*shapes[0].mesh.indices.size(),
        shapes[0].mesh.indices.data(), GL_STATIC_DRAW);

    indicesCount=shapes[0].mesh.indices.size();

    glBindVertexArray(0);


    // load shader program
    setup_shader(readfile("vs.txt").c_str(), readfile("fs.txt").c_str());
}

static void destroy()
{
    glDeleteBuffers(4, vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

static void setUniformMat4(const string &name, const glm::mat4 &mat)
{
    // This line can be ignore. But, if you have multiple shader program
    // You must check if currect binding is correct
    glUseProgram(program);
    GLint loc=glGetUniformLocation(program, name.c_str());
    if(loc==-1) return;

    // mat4 of glm is column major, same as opengl
    // we don't need to transpose it. so..GL_FALSE
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

static void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);
    glUseProgram(program);
    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        cerr<<"Usage: HW4 <obj>"<<endl;
        return 1;
    }

    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    // This line MUST put below glfwMakeContextCurrent
    glewExperimental = GL_TRUE;
    glewInit();

    // Enable vsync
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    glfwSwapBuffers(window);
    setup(argv[1]);

    glEnable(GL_DEPTH_TEST);

    setUniformMat4("model", glm::mat4(1.0f));
    setUniformMat4("mvp", glm::perspective(45.0f, 640.0f/480, 0.1f, 100.f)*
        glm::lookAt(glm::vec3(10.0f), glm::vec3(), glm::vec3(0, 1, 0))*glm::mat4(1.0f));

    while (!glfwWindowShouldClose(window))
    {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
