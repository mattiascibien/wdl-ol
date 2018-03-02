#include "lice_gl_ctx.h"

#define MAX_CACHED_GLYPHS 4096

// create one hidden window per process to hold the openGL state,
// its GL render context stays current for the life of the process,
// we serve all framebuffers from the same render context

class LICE_GL_ctx
{
public:

	LICE_GL_ctx();
	~LICE_GL_ctx();

	bool IsValid();
	HWND GetWindow() { return m_hwnd; }
	void Close();

	int GetTexFromGlyph(const unsigned char* glyph, int glyph_w, int glyph_h);
	void ClearTex();

	struct GlyphCache
	{
		unsigned int tex;
		unsigned char* glyph; // lives on the heap
		int glyph_w, glyph_h;
	};

private:

	bool Init();

	bool m_init_tried;
	HINSTANCE m_gldll;
	HWND m_hwnd;
	HGLRC m_glrc;

	GlyphCache m_glyphCache[MAX_CACHED_GLYPHS];
	int m_nCachedGlyphs;
};

LICE_GL_ctx::LICE_GL_ctx()
{
	m_init_tried = false;
	m_gldll = 0;
	m_hwnd = 0;
	m_glrc = 0;
	memset(m_glyphCache, 0, MAX_CACHED_GLYPHS * sizeof(GlyphCache));
	m_nCachedGlyphs = 0;
}

LICE_GL_ctx::~LICE_GL_ctx()
{
	Close();
}

bool LICE_GL_ctx::Init()
{
	m_init_tried = true;

	m_gldll = LoadLibrary("opengl32.dll");
	if (!m_gldll)
	{
		Close();
		return false;
	}

	// create a minimal GL render context to serve FBOs out of
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = "LICE_GL_ctx";
	RegisterClass(&wc);
	m_hwnd = CreateWindow("LICE_GL_ctx", "LICE_GL_ctx", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
	HDC dc = GetDC(m_hwnd);
	if (!dc)
	{
		Close();
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	int pxfmt = ChoosePixelFormat(dc, &pfd);
	if (!SetPixelFormat(dc, pxfmt, &pfd))
	{
		Close();
		return false;
	}

	m_glrc = wglCreateContext(dc);
	if (!wglMakeCurrent(dc, m_glrc)) // render context should stay current throughout
	{
		Close();
		return false;
	}

	/*char *rendstr = (char*)glGetString(GL_RENDERER);
	if (!rendstr || strstr(rendstr, "GDI"))
	{
		Close();
		return false;
	}*/

	// check now for all the extension functions we will ever need
	if (!gladLoadGL())
	{
		Close();
		return false;
	}

	// any one-time initialization goes here
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	ReleaseDC(m_hwnd, dc);

	return true;
}

bool LICE_GL_ctx::IsValid()
{
	if (m_gldll && m_glrc) return true;
	if (!m_init_tried) return Init();
	return false;
}

void LICE_GL_ctx::Close()
{
	ClearTex();
	if (m_glrc)
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(m_glrc);
		m_glrc = 0;
	}
	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = 0;
	}
	if (m_gldll)
	{
		FreeLibrary(m_gldll);
		m_gldll = 0;
	}
}

void LICE_GL_ctx::ClearTex()
{
	int i;
	for (i = 0; i < m_nCachedGlyphs; ++i)
	{
		glDeleteTextures(1, &m_glyphCache[i].tex);
		free(m_glyphCache[i].glyph);
		memset(&m_glyphCache[i], 0, sizeof(GlyphCache));
	}
	m_nCachedGlyphs = 0;
}

static int _glyphcmp(const void* p1, const void* p2)
{
	LICE_GL_ctx::GlyphCache* gc1 = (LICE_GL_ctx::GlyphCache*) p1;
	LICE_GL_ctx::GlyphCache* gc2 = (LICE_GL_ctx::GlyphCache*) p2;

	if (gc1->glyph_w < gc2->glyph_w) return -1;
	if (gc1->glyph_w > gc2->glyph_w) return 1;
	if (gc1->glyph_h < gc2->glyph_h) return -1;
	if (gc1->glyph_h > gc2->glyph_h) return 1;
	return memcmp(gc1->glyph, gc2->glyph, gc1->glyph_w*gc1->glyph_h);
}

int LICE_GL_ctx::GetTexFromGlyph(const unsigned char* glyph, int glyph_w, int glyph_h)
{
	if (!IsValid()) return false;

	GlyphCache gc;
	gc.tex = 0;
	gc.glyph = (unsigned char *)glyph;
	gc.glyph_w = glyph_w;
	gc.glyph_h = glyph_h;

	GlyphCache* p = (GlyphCache*)bsearch(&gc, m_glyphCache, m_nCachedGlyphs, sizeof(GlyphCache), _glyphcmp);
	if (p) return p->tex;

	glGenTextures(1, &gc.tex);
	glBindTexture(GL_TEXTURE_RECTANGLE, gc.tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_ALPHA8, glyph_w, glyph_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, glyph);

	if (m_nCachedGlyphs >= MAX_CACHED_GLYPHS) ClearTex(); // quick & dirty

	gc.glyph = (unsigned char*)malloc(glyph_w*glyph_h);
	memcpy(gc.glyph, glyph, glyph_w*glyph_h);
	m_glyphCache[m_nCachedGlyphs++] = gc; // copy
	qsort(m_glyphCache, m_nCachedGlyphs, sizeof(GlyphCache), _glyphcmp);

	return gc.tex;
}

////////

static LICE_GL_ctx s_glctx;  // one static opengl context object per process


bool LICE_GL_IsValid()
{
	return s_glctx.IsValid();
}

HWND LICE_GL_GetWindow()
{
	if (s_glctx.IsValid()) return s_glctx.GetWindow();
	return 0;
}

void LICE_GL_CloseCtx()
{
	s_glctx.Close();
}

GLuint LICE_GL_GetTexFromGlyph(const unsigned char* glyph, int glyph_w, int glyph_h)
{
	return s_glctx.GetTexFromGlyph(glyph, glyph_w, glyph_h);
}

void LICE_GL_ClearTex()
{
	s_glctx.ClearTex();
}

