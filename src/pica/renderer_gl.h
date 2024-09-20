#ifndef RENDERER_GL_H
#define RENDERER_GL_H

#include "gpu.h"

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

void renderer_gl_setup();

#endif
