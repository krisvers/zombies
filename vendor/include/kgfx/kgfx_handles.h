#ifndef KRISVERS_KGFX_HANDLES_H
#define KRISVERS_KGFX_HANDLES_H

#define KGFX_DEFINE_HANDLE(structure) typedef struct structure##_t* structure;
#define KGFX_HANDLE_NULL (0)

KGFX_DEFINE_HANDLE(KGFXcontext)

/* pipeline related handles */
KGFX_DEFINE_HANDLE(KGFXpipeline)
KGFX_DEFINE_HANDLE(KGFXframebuffer)
KGFX_DEFINE_HANDLE(KGFXshader)

/* command list */
KGFX_DEFINE_HANDLE(KGFXcommandlist)

/* buffer related handles */
KGFX_DEFINE_HANDLE(KGFXbuffer)
KGFX_DEFINE_HANDLE(KGFXtexture)
KGFX_DEFINE_HANDLE(KGFXsampler)

#endif