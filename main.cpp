#include <iostream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include <glm/vec2.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void printDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  std::cerr << message << std::endl;
}

GLuint compileShader(GLenum shaderType, const std::string& src)
{
  auto id = glCreateShader(shaderType);
  const auto srcData = src.data();
  glShaderSource(id, 1, &srcData, nullptr);
  glCompileShader(id);
  return id;
};

template < typename Container >
static const GLuint makeBuffer(const Container& data )
{
  GLuint ret;
  glGenBuffers(1, &ret);
  glBindBuffer(GL_ARRAY_BUFFER, ret);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_STATIC_DRAW);
  return ret;
}

int main(int argc, const char* argv[])
{
  std::ignore = argc;
  std::ignore = argv;

  // Init
  //@{

  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))
  {
    std::cout << "Could not init SDL" << std::endl;
    return 1;
  }

  if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
  {
    std::cout << "Warning: Could not enable vsync" << std::endl;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  const auto window = SDL_CreateWindow("opengl-blend-demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  const auto context = SDL_GL_CreateContext(window);

  glewExperimental = GL_TRUE;
  if (glewInit())
  {
    std::cout << "Error: Could not init GLEW" << std::endl;
    return 1;
  }

  glDebugMessageCallback(&printDebugMessage, nullptr);

  //@}
  // Load shaders
  //@{

  const auto vertexShadersSrc = R"""(
#version 330

in vec2 position;
in vec2 uv;

out vec2 vertex_uv;

void main()
{
  gl_Position = vec4(position, 0.0f, 1.0f);
  vertex_uv = uv;
}
  )""";
  auto vertexShaderId = compileShader(GL_VERTEX_SHADER, vertexShadersSrc);
  const auto fragmentShaderSrc = R"""(
#version 330

in vec2 vertex_uv;

out vec4 color;

uniform sampler2D sampler;

void main()
{
  color = texture(sampler, vertex_uv);
}
  )""";
  auto fragmentShaderId = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

  auto program = glCreateProgram();
  glAttachShader(program, vertexShaderId);
  glAttachShader(program, fragmentShaderId);
  glLinkProgram(program);
  glUseProgram(program);

  //@}
  // Create geometry
  //@{

  GLuint vertexArrayId;
  glGenVertexArrays(1, &vertexArrayId);
  glBindVertexArray(vertexArrayId);

  const auto vertexBuffer = makeBuffer(std::vector<glm::vec2>{
    {-1.0f, -1.0f},
    { 1.0f, -1.0f},
    {-1.0f,  1.0f},
    { 1.0f,  1.0f},
  });
  const auto uvBuffer = makeBuffer(std::vector<glm::vec2>{
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f},
  });

  //@}
  // Texture
  //@{

  std::vector<GLuint> textureIds(argc - 1, 0);
  glGenTextures(textureIds.size(), textureIds.data());
  for (int i = 0; i < textureIds.size(); ++i)
  {
    glBindTexture(GL_TEXTURE_2D, textureIds[i]);
    int width, height, n;
    const auto data = stbi_load(argv[i+1], &width, &height, &n, 4);
    if (data != nullptr)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
  }

  //@}
  // Configuration
  //@{

  GLenum sfactor = GL_SRC_ALPHA;
  GLenum dfactor = GL_ONE_MINUS_SRC_ALPHA;
  GLenum modeRgb = GL_FUNC_ADD;
  GLenum modeAlpha = GL_FUNC_ADD;

  const std::unordered_map<SDL_Keycode, GLenum> keyToBlendFunc{
    {SDLK_q, GL_ZERO},
    {SDLK_w, GL_ONE},
    {SDLK_e, GL_SRC_COLOR},
    {SDLK_r, GL_ONE_MINUS_SRC_COLOR},
    {SDLK_t, GL_DST_COLOR},
    {SDLK_y, GL_ONE_MINUS_DST_COLOR},
    {SDLK_u, GL_SRC_ALPHA},
    {SDLK_i, GL_ONE_MINUS_SRC_ALPHA},
    {SDLK_o, GL_DST_ALPHA},
    {SDLK_p, GL_ONE_MINUS_DST_ALPHA},
    {SDLK_a, GL_CONSTANT_COLOR},
    {SDLK_s, GL_ONE_MINUS_CONSTANT_COLOR},
    {SDLK_d, GL_CONSTANT_ALPHA},
    {SDLK_f, GL_ONE_MINUS_CONSTANT_ALPHA},
  };
  const std::unordered_map<SDL_Keycode, GLenum> keyToBlendEquation{
    {SDLK_z, GL_FUNC_ADD},
    {SDLK_x, GL_FUNC_SUBTRACT},
    {SDLK_c, GL_FUNC_REVERSE_SUBTRACT},
    {SDLK_v, GL_MIN},
    {SDLK_b, GL_MAX},
  };

  //@}
  // Main loop
  //@{

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  bool run = true;
  while (run)
  {
    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_KEYDOWN:
          if (keyToBlendFunc.count(event.key.keysym.sym))
          {
            auto factor = &sfactor;
            if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LSHIFT])
            {
              factor = &dfactor;
            }
            *factor = keyToBlendFunc.at(event.key.keysym.sym);
          }
          else if (keyToBlendEquation.count(event.key.keysym.sym))
          {
            auto mode = &modeRgb;
            if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LSHIFT])
            {
              mode = &modeAlpha;
            }
            *mode = keyToBlendEquation.at(event.key.keysym.sym);
          }
          break;

        case SDL_QUIT:
          run = false;
          break;

        case SDL_WINDOWEVENT:
          switch (event.window.event)
          {
          case SDL_WINDOWEVENT_RESIZED:
            glViewport(0, 0, event.window.data1, event.window.data2);
            break;

          default:
            break;
          }
          break;

        default:
          break;
      }
    }

    // Render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLuint positionIndex = 0;
    const GLuint uvIndex = 1;

    glEnableVertexAttribArray(positionIndex);
    glEnableVertexAttribArray(uvIndex);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(positionIndex, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBlendFunc(sfactor, dfactor);
    glBlendEquationSeparate(modeRgb, modeAlpha);

    for (int i = 0; i < textureIds.size(); ++i)
    {
      glBindTexture(GL_TEXTURE_2D, textureIds[i]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    SDL_GL_SwapWindow(window);
  }

  //@}

  SDL_GL_DeleteContext(context);

  return 0;
}
