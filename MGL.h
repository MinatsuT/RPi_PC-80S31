//
// Minatsu Game Library by Minatsu
//
#ifndef __MGL_THREAD_H_
#define __MGL_THREAD_H_

#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

#include "vec2d.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MGLDEF
#ifdef MGL_STATIC
#define MGLDEF static
#else
#define MGLDEF extern
#endif
#endif

// Parameters
#define SW SCREEN_WIDTH
#define SH SCREEN_HEIGHT
#define GRP_N 4
// #define GRP_W (2048 / 2)
// #define GRP_H (2048 / 2)
#define GRP_W (640)
#define GRP_H (200)
#define BG_N 4
#define BG_W 512
#define BG_H 512
#define SP_N 1

// Utility macros
//--------------------------------------------------------------------------------
#define RNDF(a) ((float)rand() / (float)RAND_MAX * (float)(a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ZEROFILL(var) memset(&(var), 0, sizeof(var))
#define SCR_X(a) ((a) / SW * 2.0 - 1.0)
#define SCR_Y(a) (-((a) / SH * 2.0 - 1.0))
#define FB_X(a) ((a) / GRP_W * 2.0 - 1.0)
#define FB_Y(a) (-((a) / GRP_H * 2.0 - 1.0))
#define TEX_U(a) ((a) / (float)GRP_W)
#define TEX_V(a) ((a) / (float)GRP_H)
#define GET_ALOC(v, p, n)                                                                      \
    do {                                                                                       \
        v = glGetAttribLocation(p, n);                                                         \
        if (v < 0) {                                                                           \
            fprintf(stderr, "GET_ALOC fail: %s:%d (in %s())\n", __FILE__, __LINE__, __func__); \
            MGL_Quit();                                                                        \
        }                                                                                      \
    } while (0)
#define GET_ULOC(v, p, n)                                                                      \
    do {                                                                                       \
        v = glGetUniformLocation(p, n);                                                        \
        if (v < 0) {                                                                           \
            fprintf(stderr, "GET_ULOC fail: %s:%d (in %s())\n", __FILE__, __LINE__, __func__); \
            MGL_Quit();                                                                        \
        }                                                                                      \
    } while (0)

// System variables
//--------------------------------------------------------------------------------
MGLDEF int maincnt;
int maincnt = 0;

// VRAM
//--------------------------------------------------------------------------------
MGLDEF uint16_t *vram[];
uint16_t *vram[GRP_N];

// Color
//--------------------------------------------------------------------------------
typedef struct {
    GLfloat r, g, b, a;
} G_RGBA;

MGLDEF G_RGBA GCol_RED;
MGLDEF G_RGBA GCol_GREEN;
MGLDEF G_RGBA GCol_BLUE;

G_RGBA GCol_RED = {1.0f, 0.0f, 0.0f, 1.0f};
G_RGBA GCol_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
G_RGBA GCol_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};

#define G_RED (&GCol_RED)
#define G_GREEN (&GCol_GREEN)
#define G_BLUE (&GCol_BLUE)

// BG
//--------------------------------------------------------------------------------
typedef struct {
    vec2d pos;
    float z;
    vec2d uv;
    vec2d wh;
    vec2d home;
    vec2d scale;
    GLfloat rot;
    GLuint page;
    G_RGBA *buf;
    uint8_t enable : 1;
    uint8_t visible : 1;
} G_BG;

#define G_BG_PAGE_DEFAULT MAX(0, (GRP_N - 1))

// SP
//--------------------------------------------------------------------------------
typedef struct {
    vec2d pos;
    float z;
    vec2d uv;
    vec2d wh;
    vec2d home;
    vec2d scale;
    float rot;
    uint8_t page;
    uint8_t enable : 1;
    uint8_t visible : 1;
} G_SP;

#define G_SP_PAGE_DEFAULT MAX(0, (GRP_N - 2))

// Blend modes
//--------------------------------------------------------------------------------
#define G_BLEND_SET glBlendFunc(GL_ONE, GL_ZERO)
#define G_BLEND_ALPHA glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
#define G_BLEND_ADD glBlendFunc(GL_ONE, GL_ONE)
#define G_BLEND_SCREEN glBlendFunc(GL_SRC_ALPHA, GL_ONE)

// Prototypes
//--------------------------------------------------------------------------------
MGLDEF int InitShader(GLuint *program, char const *vShSrc, char const *fShSrc);
MGLDEF GLuint LoadShader(GLenum type, const char *shaderSource);
MGLDEF int timemillis(void);
MGLDEF void vsync(void);
MGLDEF void check_errors(const char *);
MGLDEF int MGL_Init();
MGLDEF void draw_and_vsync(void);
MGLDEF int MGL_SDL_Init();
MGLDEF int MGL_Init(void);
MGLDEF void MGL_Quit(void);
MGLDEF int main_loop(void);
MGLDEF void finalize(void);
MGLDEF uint32_t rgb(int r, int g, int b);
MGLDEF uint32_t rgba(int r, int g, int b, int a);
MGLDEF int spset(int n, float u, float v, float w, float h);
MGLDEF int sphome(int n, float hx, float hy);
MGLDEF int sprot(int n, float rot);
MGLDEF int sppage(int n, int page);
MGLDEF int sphide(int n);
MGLDEF int spshow(int n);
MGLDEF int spclear(int n);
MGLDEF void gtarget(int target);
MGLDEF void gline(float x1, float y1, float x2, float y2, uint32_t c);
MGLDEF void gpset(float x1, float y1, uint16_t c);
MGLDEF uint32_t rgb(int r, int g, int b);
MGLDEF uint32_t rgba(int r, int g, int b, int a);
MGLDEF void bgcolor(uint32_t c);
MGLDEF int gload(const char *fname, int gpage, int ox, int oy);

//================================================================================
/////////////////////////////   end header file   ////////////////////////////////
//================================================================================
#endif // __MGL_THREAD_H_
#ifdef MGL_IMPLEMENTATION

//================================================================================
// Debug support facilities
//================================================================================
#define Debug 1
#if defined(Debug)
#define DP(...) printf(__VA_ARGS__)
#else
#define DP(...)
#endif

// Private variables
static SDL_Window *wnd = NULL;
static SDL_Surface *surface = NULL;

static GLuint grp[GRP_N];
static GLuint fb[GRP_N];
static G_BG bg[BG_N];
static G_SP sp[SP_N];

static pthread_mutex_t vram_change_mtx;
static int vram_change_flag[GRP_N];

//================================================================================
// BG
//================================================================================
const char vBGShaderSrc[] = "#version 100 \n"
                            "attribute vec2 aPos; \n"
                            "attribute vec2 aTex; \n"
                            "varying vec2 vTex; \n"
                            "void main() \n"
                            "{ \n"
                            " vTex = aTex; \n"
                            " gl_Position = vec4(aPos, 0.0, 1.0); \n"
                            "} \n";

const char fBGShaderSrc[] = "#version 100 \n"
                            "precision mediump float; \n"
                            "varying vec2 vTex; \n"
                            "uniform sampler2D uTexture; \n" // GRPの画像
                            "uniform sampler2D uBG; \n"      // BGマップ定義
                            "uniform float uBG_size; \n"
                            "uniform float uTexture_size; \n"
                            "uniform float uTile_size; \n"
                            "void main() \n"
                            "{ \n"
                            " vec2 bgCoord = vec2(vTex.s,vTex.t)*uBG_size; \n"   // BG画面上の座標に変換
                            " vec2 bgIdx = floor(bgCoord); \n"                   // 整数部分：BG画面のタイルを単位とした座標
                            " vec2 bgDecimal = (bgCoord - bgIdx)*0.99+0.005; \n" // 小数部分：タイル内の座標(少し内側に寄せる)
                            " vec4 bgNum = texture2D(uBG, (bgIdx+0.5)/uBG_size); \n" // BGマップ定義からタイル番号を取得(正確には、赤成分=X,緑成分=Y)
                            " vec2 texPos = (floor(bgNum.rg*255.0+0.5)+bgDecimal)*uTile_size; \n" // GRP上での座標を計算
                            " gl_FragColor = texture2D(uTexture, texPos/uTexture_size); \n"       // 色を出力
                            "} \n";
static GLuint prgBG = 0;
static int prgBG_aPos;
static int prgBG_aTex;
static int prgBG_uTexture;
static int prgBG_uBG;
static int prgBG_uBG_size;
static int prgBG_uTexture_size;
static int prgBG_uTile_size;

//================================================================================
// SP
//================================================================================
const char vSPShaderSrc[] = "#version 100 \n"
                            "attribute vec2 aPos; \n"
                            "attribute vec2 aTex; \n"
                            "varying vec2 vTex; \n"
                            "void main() \n"
                            "{ \n"
                            " vTex = aTex; \n"
                            " gl_Position = vec4(aPos, 0.0, 1.0); \n"
                            "} \n";

const char fSPShaderSrc[] = "#version 100 \n"
                            "precision mediump float; \n"
                            "varying vec2 vTex; \n"
                            "uniform sampler2D uTexture; \n"
                            "void main() \n"
                            "{ \n"
                            " gl_FragColor = texture2D(uTexture, vTex); \n"
                            "} \n";
static GLuint prgSP = 0;
static int prgSP_aPos;
static int prgSP_aTex;
static int prgSP_uTexture;

// set SP
int spset(int n, float u, float v, float w, float h) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }

    sp[n].pos.x = 0;
    sp[n].pos.y = 0;
    sp[n].z = 0;
    sp[n].uv.x = u;
    sp[n].uv.y = v;
    sp[n].wh.x = w;
    sp[n].wh.y = h;
    sp[n].home.x = 0;
    sp[n].home.y = 0;
    sp[n].rot = 0;
    sp[n].scale.x = 1;
    sp[n].scale.y = 1;
    sp[n].page = G_SP_PAGE_DEFAULT;
    sp[n].enable = 1;
    sp[n].visible = 1;

    return 0;
}

// set SP offset
int spofs(int n, float x, float y, float z) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].pos.x = x;
    sp[n].pos.y = y;
    sp[n].z = z;
    return 0;
}

// set SP home position
int sphome(int n, float hx, float hy) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].home.x = hx;
    sp[n].home.y = hy;
    return 0;
}

// set SP rotation angle
int sprot(int n, float rot) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].rot = rot;
    return 0;
}

// set SP scale
int spscale(int n, float sx, float sy) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].scale.x = sx;
    sp[n].scale.y = sy;
    return 0;
}

// set SP texture page
int sppage(int n, int page) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    if (page < 0 || page >= GRP_N - 1) {
        return -1;
    }
    sp[n].page = page;
    return 0;
}

// hide SP
int sphide(int n) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].visible = 0;
    return 0;
}

// show SP
int spshow(int n) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    sp[n].visible = 1;
    return 0;
}

// clear SP
int spclear(int n) {
    if (n < 0 || n >= SP_N) {
        return -1;
    }
    ZEROFILL(sp[n]);
    return 0;
}

//================================================================================
// Graphics
//================================================================================
const char vGShaderSrc[] = "#version 100 \n"
                           "attribute vec2 aPos; \n"
                           "void main() \n"
                           "{ \n"
                           " gl_Position = vec4(aPos, 0.0, 1.0); \n"
                           "} \n";

const char fGShaderSrc[] = "#version 100 \n"
                           "precision mediump float; \n"
                           "uniform vec4 uCol; \n"
                           "void main() \n"
                           "{ \n"
                           " gl_FragColor = uCol; \n"
                           //" gl_FragColor = vec4(uCol.r,uCol.g,uCol.b,1.0); \n"
                           "} \n";
static GLuint prgG = 0;
static int prgG_aPos;
static int prgG_uCol;
static int g_target = 0;
static G_RGBA g_bgcolor = {0.0f, 0.0f, 0.0f, 1.0f};

// set target GRP
void gtarget(int target) {
    if (target < 0 || target > GRP_N - 1) {
        return;
    }
    g_target = target;
}

// draw line
void gline(float x1, float y1, float x2, float y2, uint32_t c) {}

// draw point
void gpset(float x1, float y1, uint16_t c) {
    // if (x1<0 || x1>=GRP_W || y1<0 || y1>=GRP_H) {
    // return;
    // }
    int p = y1 * GRP_W + x1;
    vram[g_target][p] = c;
    // pthread_mutex_lock(&vram_change_mtx);
    // vram_change_flag[g_target] = 1;
    // pthread_mutex_unlock(&vram_change_mtx);
}

// RGB
uint32_t rgb(int r, int g, int b) {
    return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | ((b & 0xff) << 0);
}
uint32_t rgba(int r, int g, int b, int a) {
    return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | ((b & 0xff) << 0);
}

// BGcolor
void bgcolor(uint32_t c) {
    g_bgcolor.r = ((c >> 16) & 0xff) / 255.0;
    g_bgcolor.g = ((c >> 8) & 0xff) / 255.0;
    g_bgcolor.b = ((c >> 0) & 0xff) / 255.0;
    g_bgcolor.a = ((c >> 24) & 0xff) / 255.0;
}

//================================================================================
// gload
//================================================================================

// load image
int gload(const char *fname, int gpage, int ox, int oy) {
    int ret = 0, w, h, n;

    if (gpage < 0 || gpage > GRP_N - 1) {
        fprintf(stderr, "GRP page is out of range: %d\n", gpage);
        return -1;
    }

    unsigned char *png = stbi_load(fname, &w, &h, &n, 0);
    if (!png) {
        fprintf(stderr, "Failed to load image: %s [%s]\n", stbi_failure_reason(), fname);
        return -1;
    }

    if (n == 4 || n == 3) {
        int fmt = (n == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, grp[gpage]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, w, h, fmt, GL_UNSIGNED_BYTE, png);
        check_errors("gload");
    } else {
        fprintf(stderr, "Unsupported number of image channels: %d [%s]\n", n, fname);
        ret = -1;
    }
    stbi_image_free(png);

    return ret;
}

//================================================================================
// Shader
//================================================================================
GLuint LoadShader(GLenum type, const char *shaderSource) {
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);
    if (shader == 0)
        return 0;

    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) { // compile error
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int InitShader(GLuint *program, char const *vShSrc, char const *fShSrc) {
    GLuint vShader;
    GLuint fShader;
    GLint linked;
    GLuint prog;

    vShader = LoadShader(GL_VERTEX_SHADER, vShSrc);
    fShader = LoadShader(GL_FRAGMENT_SHADER, fShSrc);

    prog = glCreateProgram();
    if (prog == 0)
        return 0;

    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) { // error
        GLint infoLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(prog, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteProgram(prog);
        return GL_FALSE;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    *program = prog;
    return GL_TRUE;
}

//================================================================================
// Utilities
//================================================================================

// get current time in milliseconds
int timemillis() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (now.tv_sec * 1000) + (now.tv_nsec / 1000000);
}

// Vsync
static pthread_cond_t vsync_cond;
static pthread_mutex_t vsync_mtx;
void vsync() {
    pthread_cond_wait(&vsync_cond, &vsync_mtx);
}

//==================================================
// Error handler
//==================================================
static char const *gl_error_string(GLenum const err) {
    switch (err) {
    // OpenGL ES 2.0 errors (6)
    case GL_NO_ERROR:
        return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";

    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";

    default:
        return "unknown error";
    }
}

void check_errors(const char *tag) {
    int err_flag = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("[%s]OpenGL ERROR code %d: %s\n", tag, err, (const char *)gl_error_string(err));
        err_flag = 1;
    }
    if (err_flag) {
        MGL_Quit();
    }
}

//================================================================================
// Renderer
//================================================================================
typedef struct {
    GLfloat x, y;
    GLfloat s, t;
} G_Vtx;
static G_Vtx sp_vtx[4];
static GLushort sp_vtx_idx[] = {0, 1, 2, 1, 3, 2};
static int trans_count = 0;
void draw_and_vsync() {
    // Copy vram to texture
    // for (int i = 0; i < GRP_N; i++) {
    trans_count = (trans_count + 1) % 2;
    if (!trans_count) {
        // int i = 2;
        // int i = trans_count;
        // if (!vram_change_flag[i]) {
        //     continue;
        // }
        // pthread_mutex_lock(&vram_change_mtx);
        // vram_change_flag[i] = 0;
        // pthread_mutex_unlock(&vram_change_mtx);
        // glTexSubImage2D(grp[i], 0, 0, 0, GRP_W, GRP_H, GL_RGBA, GL_UNSIGNED_BYTE, vram[i]);
        glBindTexture(GL_TEXTURE_2D, grp[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRP_W, GRP_H, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, vram[0]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SW, SH);
    glClearColor(g_bgcolor.r, g_bgcolor.g, g_bgcolor.b, g_bgcolor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    G_BLEND_ALPHA;

    // SP
    for (int i = 0; i < SP_N; i++) {
        if (!sp[i].enable || !sp[i].visible) {
            continue;
        }

        G_SP *s = &sp[i];
        vec2d p;

        // 左上
        p.x = -s->home.x;
        p.y = -s->home.y;
        p.x *= s->scale.x;
        p.y *= s->scale.y;
        p = v2add(v2rotd(p, s->rot), s->pos);
        sp_vtx[0].x = SCR_X(p.x);
        sp_vtx[0].y = SCR_Y(p.y);
        sp_vtx[0].s = TEX_U(sp[i].uv.x);
        sp_vtx[0].t = TEX_V(sp[i].uv.y);
        // 右上
        p.x = s->wh.x - s->home.x;
        p.y = -s->home.y;
        p.x *= s->scale.x;
        p.y *= s->scale.y;
        p = v2add(v2rotd(p, s->rot), s->pos);
        sp_vtx[1].x = SCR_X(p.x);
        sp_vtx[1].y = SCR_Y(p.y);
        sp_vtx[1].s = TEX_U(sp[i].uv.x + sp[i].wh.x);
        sp_vtx[1].t = TEX_V(sp[i].uv.y);
        // 左下
        p.x = -s->home.x;
        p.y = s->wh.y - s->home.y;
        p.x *= s->scale.x;
        p.y *= s->scale.y;
        p = v2add(v2rotd(p, s->rot), s->pos);
        sp_vtx[2].x = SCR_X(p.x);
        sp_vtx[2].y = SCR_Y(p.y);
        sp_vtx[2].s = TEX_U(sp[i].uv.x);
        sp_vtx[2].t = TEX_V(sp[i].uv.y + sp[i].wh.y);
        // 右下
        p.x = s->wh.x - s->home.x;
        p.y = s->wh.y - s->home.y;
        p.x *= s->scale.x;
        p.y *= s->scale.y;
        p = v2add(v2rotd(p, s->rot), s->pos);
        sp_vtx[3].x = SCR_X(p.x);
        sp_vtx[3].y = SCR_Y(p.y);
        sp_vtx[3].s = TEX_U(sp[i].uv.x + sp[i].wh.x);
        sp_vtx[3].t = TEX_V(sp[i].uv.y + sp[i].wh.y);

        // printf("左上: (%f, %f) (%f, %f)\n", sp_vtx[0].x, sp_vtx[0].y, sp_vtx[0].s, sp_vtx[0].t);
        // printf("右上: (%f, %f) (%f, %f)\n", sp_vtx[1].x, sp_vtx[1].y, sp_vtx[1].s, sp_vtx[1].t);
        // printf("左下: (%f, %f) (%f, %f)\n", sp_vtx[2].x, sp_vtx[2].y, sp_vtx[2].s, sp_vtx[2].t);
        // printf("右下: (%f, %f) (%f, %f)\n", sp_vtx[3].x, sp_vtx[3].y, sp_vtx[3].s, sp_vtx[3].t);

        glUseProgram(prgSP);
        glUniform1i(prgSP_uTexture, 0); // ←GL_TEXTURE0 の「0」
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grp[sp[i].page]);

        glVertexAttribPointer(prgSP_aPos, 2, GL_FLOAT, GL_FALSE, sizeof(G_Vtx), ((void *)sp_vtx) + sizeof(GLfloat) * 0);
        glEnableVertexAttribArray(prgSP_aPos);
        glVertexAttribPointer(prgSP_aTex, 2, GL_FLOAT, GL_FALSE, sizeof(G_Vtx), ((void *)sp_vtx) + sizeof(GLfloat) * 2);
        glEnableVertexAttribArray(prgSP_aTex);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, sp_vtx_idx);
    }

#if 0
    // BG
    glBindFramebuffer(GL_FRAMEBUFFER, NULL);
    glUseProgram(g_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grp[0]);
    glUniform1i(glGetUniformLocation(g_program, "uTexture"), 0); // ←GL_TEXTURE0 の「0」
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bg[0]);
    glUniform1i(glGetUniformLocation(g_program, "uBG"), 1); // ←GL_TEXTURE1 の「1」
    glUniform1f(glGetUniformLocation(g_program, "uBG_size"), (float)BG_W);
    glUniform1f(glGetUniformLocation(g_program, "uTexture_size"), (float)GRP_W);
    glUniform1f(glGetUniformLocation(g_program, "uTile_size"), (float)16.0);

    int aPos = glGetAttribLocation(g_program, "aPos");
    glVertexAttribPointer(aPos, 2, GL_FLOAT, GL_FALSE, sizeof(VtxType), ((void *)vObj) + sizeof(GLfloat) * 0);
    glEnableVertexAttribArray(aPos);

    int aTex = glGetAttribLocation(g_program, "aTex");
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, sizeof(VtxType), ((void *)vObj) + sizeof(GLfloat) * 2);
    glEnableVertexAttribArray(aTex);

    glViewport(0, 0, width, height);
    // glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    check_errors("CLEAR");
#endif

    SDL_GL_SwapWindow(wnd);
}
//================================================================================
// Initializer
//================================================================================
// SDL
//--------------------------------------------------------------------------------
int MGL_SDL_Init() {
    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return -1;
    }

    // ウィンドウの作成
    wnd = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (wnd == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return -1;
    }

    // OpenGLコンテキストの作成
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(wnd);

    SDL_GL_SetSwapInterval(1);

    // OpenGLのバージョン表示
    DP("Renderer: %s\n", glGetString(GL_RENDERER));
    DP("Version: %s\n", glGetString(GL_VERSION));
    DP("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    DP("Vendor: %s\n", glGetString(GL_VENDOR));

    return 0;
}

// MGL
//--------------------------------------------------------------------------------
int MGL_Init() {
    // SDL
    if (MGL_SDL_Init() < 0) {
        perror("Failed to init SDL.");
        return -1;
    }

    // シェーダーの初期化
    // Graphics shader
    if (!prgG && !InitShader(&prgG, vGShaderSrc, fGShaderSrc)) {
        perror("Cannot init Graphics Shader.");
        return -1;
    }
    GET_ALOC(prgG_aPos, prgG, "aPos");
    GET_ULOC(prgG_uCol, prgG, "uCol");

    // BG shader
    if (!prgBG && !InitShader(&prgBG, vBGShaderSrc, fBGShaderSrc)) {
        perror("Cannot init BG Shader.");
        return -1;
    }
    GET_ALOC(prgBG_aPos, prgBG, "aPos");
    GET_ALOC(prgBG_aTex, prgBG, "aTex");
    GET_ULOC(prgBG_uTexture, prgBG, "uTexture");
    GET_ULOC(prgBG_uBG, prgBG, "uBG");
    GET_ULOC(prgBG_uBG_size, prgBG, "uBG_size");
    GET_ULOC(prgBG_uTexture_size, prgBG, "uTexture_size");
    GET_ULOC(prgBG_uTile_size, prgBG, "uTile_size");

    // SP shader
    if (!prgSP && !InitShader(&prgSP, vSPShaderSrc, fSPShaderSrc)) {
        perror("Cannot init SP Shader.");
        return -1;
    }
    GET_ALOC(prgSP_aPos, prgSP, "aPos");
    GET_ALOC(prgSP_aTex, prgSP, "aTex");
    GET_ULOC(prgSP_uTexture, prgSP, "uTexture");

    // グラフィックページ(GRP)を作成
    ZEROFILL(vram);
    ZEROFILL(vram_change_flag);
    for (int i = 0; i < GRP_N; i++) {
        // テクスチャ
        // --------------------------------------------------------------------------------
        DP("GRP%d...", i);
        int vram_size_n = GRP_W * GRP_H;
        vram[i] = calloc(sizeof(*vram[0]), vram_size_n);
        if (!vram[i]) {
            fprintf(stderr, "Cannot allocate vram (%dbytes) for GRP%d\n", sizeof(*vram[0]) * vram_size_n, i);
            return -1;
        }

        glGenTextures(1, &grp[i]);
        check_errors("MGL_Init: glGenTextures");
        glBindTexture(GL_TEXTURE_2D, grp[i]);
        check_errors("MGL_Init: glBindTexture");
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        check_errors("MGL_Init: glPixelStorei");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        check_errors("MGL_Init: glTexParameteri 1");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        check_errors("MGL_Init: glTexParameteri 2");
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRP_W, GRP_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, grpPtr[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRP_W, GRP_H, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, vram[i]);
        check_errors("MGL_Init: glTexImage2D");

        // フレームバッファ
        // --------------------------------------------------
        glGenFramebuffers(1, &fb[i]);
        check_errors("MGL_Init: glGenFramebuffers");
        glBindFramebuffer(GL_FRAMEBUFFER, fb[i]);
        check_errors("MGL_Init: glBindFramebuffer");
        glBindTexture(GL_TEXTURE_2D, grp[i]);
        check_errors("MGL_Init: glBindTexture");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, grp[i], 0);
        check_errors("MGL_Init: glFramebufferTexture2D");

        GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (stat != GL_FRAMEBUFFER_COMPLETE) {
            DP("NG. glCheckFramebufferStatus=%d\n", stat);
            return -1;
        }
        DP("OK\n");

        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glEnable(GL_BLEND);
    glLineWidth(1.0f);

    // SP, BGの初期化
    ZEROFILL(sp);
    ZEROFILL(bg);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(wnd);

    return 0;
}

//================================================================================
// Main
//================================================================================
static pthread_t g_th;
static volatile int thread_run_flag = 1;
static volatile int sdl_ready = 0;

// Signal handler
//--------------------------------------------------------------------------------
volatile sig_atomic_t main_run_flag = 1;
void sigintHandler(int sig) {
    main_run_flag = 0;
    MGL_Quit();
}

//--------------------------------------------------------------------------------
// Finalizer
//--------------------------------------------------------------------------------
void MGL_Quit() {
    puts("Main: Waiting for thread finished...");
    thread_run_flag = 0;
    if (pthread_join(g_th, NULL) != 0) {
        perror("Main: Failed to join thread.");
    } else {
        puts("Main: joined.");
    }

    SDL_DestroyWindow(wnd);
    SDL_Quit();

    // Release vrams
    for (int i = 0; i < GRP_N; i++) {
        if (vram[i]) {
            free(vram[i]);
        }
    }

    puts("Main: Quit SDL.");
    exit(0);
}

//--------------------------------------------------------------------------------
// Screen rendering thread
//--------------------------------------------------------------------------------
void *thread_run(void *arg) {
    puts("Thread: Start rendering thread.");
    if (MGL_Init() < 0) {
        perror("Failed to init MGL.");
        exit(1);
    }
    sdl_ready = 1;

    while (thread_run_flag) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    break;
                case SDLK_RIGHT:
                    break;
                case SDLK_DOWN:
                    break;
                case SDLK_LEFT:
                    break;
                case SDLK_ESCAPE:
                    puts("Thread: Send SIGINT.");
                    pid_t pid = getpid();
                    kill(pid, SIGINT);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        draw_and_vsync();
        pthread_cond_signal(&vsync_cond);
        maincnt++;
        if (!(maincnt % 60)) {
            printf("Thead: %d\r", maincnt / 60);
        }
    }

    puts("Thead: Screen rendering thread finished.");
}

//--------------------------------------------------------------------------------
// Startup
//--------------------------------------------------------------------------------
int MGL_Start() {
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    // Adding signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("Failed to add signal handler");
        return 1;
    }

    // スレッドの作成・開始
    if (pthread_create(&g_th, NULL, thread_run, NULL) != 0) {
        perror("Failed to start thread");
        return 1;
    }

    while (!sdl_ready) {
        SDL_Delay(10);
    }

    puts("MGL init finished.\n");
    return 0;
}

#endif // MGL_IMPLEMENTATION
#ifdef __cplusplus
}
#endif
