#ifndef KRISVERS_KGFX_GH_H
#define KRISVERS_KGFX_GH_H

#include <kgfx/kgfx_structures.h>
#include <kgfx/kgfx_platform.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef KGFX_GH_API
#define KGFX_GH_API
#endif

extern "C" KGFX_GH_API KGFXwindow kgfxWindowFromGLFW(GLFWwindow * window);

#endif
