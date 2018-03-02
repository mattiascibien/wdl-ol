#ifndef _GL_CTX_
#define _GL_CTX_

#include "lice.h"

#include "glad/include/glad/glad.h"

// GL context functions
// opening and managing GL context is handled behind the scenes


bool LICE_GL_IsValid(); // GL context is initialized (will be lazy initialized on first call) and valid

HWND LICE_GL_GetWindow(); // Get the window that owns the GL context (one per process)

void LICE_GL_CloseCtx();  // Something failed, turn off GL context forever so we don't keep failing

// facility for associating a glyph with a texture
GLuint LICE_GL_GetTexFromGlyph(const unsigned char* glyph, int glyph_w, int glyph_h); 
void LICE_GL_ClearTex();

#endif
