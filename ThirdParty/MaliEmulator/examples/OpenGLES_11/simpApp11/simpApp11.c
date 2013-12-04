/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2007, 2009, 2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/* file simpApp11.c
 * Color Cube example using OpenGL ES 1.1 on Linux Desktop
 */

#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static const GLfloat coord [] = {
				  1.f,  1.f,  1.f,      /* 0 */
				 -1.f,  1.f,  1.f,      /* 1 */
				 -1.f, -1.f,  1.f,      /* 2 */
				  1.f, -1.f,  1.f,      /* 3 */
				 -1.f,  1.f, -1.f,      /* 4 */
				  1.f,  1.f, -1.f,      /* 5 */
				  1.f, -1.f, -1.f,      /* 6 */
				 -1.f, -1.f, -1.f };    /* 7 */

static const GLubyte color [] = {
				 255, 255, 255, 255, 
				 255,   0, 255, 255, 
				 0,     0, 255, 255, 
				 0,   255, 255, 255,
				 255,   0,   0, 255,
				 255, 255,   0, 255,
				 0,   255,   0, 255,
				 0,     0,   0, 255};

static const GLushort fanIx [] = {
				 0, 1, 2, 3, 6, 5, 4, 1,   /* One tri.fan  */
				 7, 6, 3, 2, 1, 4, 5, 6};  /*  and another */

/* Waits for map notification */
Bool WaitForMap(Display *d, XEvent *e, char *win_ptr)
{
    if (e->type == MapNotify && e->xmap.window == (*((Window*)win_ptr)))
    {
        return True;
    }
    return False;
}


/* Creates an X window */
Window CreateXWindow(
        const char *title,
        int width,
        int height,
        Display* display,
        EGLDisplay sEGLDisplay,
        EGLConfig FBConfig,
        Colormap *pColormap,
        XVisualInfo **ppVisual)
{
    XSetWindowAttributes wa;
    XSizeHints sh;
    XEvent e;
    unsigned long mask;
    long screen;
    XVisualInfo *visual, template;
    Colormap colormap;
    int vID, n;
    Window window;

    screen = DefaultScreen(display);
    eglGetConfigAttrib(sEGLDisplay, FBConfig, EGL_NATIVE_VISUAL_ID, &vID);
    template.visualid = vID;
    visual = XGetVisualInfo(display, VisualIDMask, &template, &n);
    colormap = XCreateColormap(
            display,
            RootWindow(display, screen),
            visual->visual,
            AllocNone);
    wa.colormap = colormap;
    wa.background_pixel = 0xFFFFFFFF;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | ExposureMask;

    mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    window = XCreateWindow(
            display,
            RootWindow(display, screen),
            0,
            0,
            width,
            height,
            0,
            visual->depth,
            InputOutput,
            visual->visual,
            mask,
            &wa);

    sh.flags = USPosition;
    sh.x = 10;
    sh.y = 10;
    XSetStandardProperties(display, window, title, title, None, 0, 0, &sh);
    XMapWindow(display, window);
    XIfEvent(display, &e, WaitForMap, (char*)&window);
    XSetWMColormapWindows(display, window, &window, 1);
    XFlush(display);

    *pColormap = colormap;
    *ppVisual = visual;

    return window;
}


static void init(void) {
    glClearColor(0.3f, 0.2f, 0.1f, 1.0f);
    glClearDepthf(1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 256, 256);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-0.6f, 0.6f, -0.6f, 0.6f, 2.0f, 10.f);
    glMatrixMode(GL_MODELVIEW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, coord);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
}


static float tim(void) {
    return (float)clock()/(float)CLOCKS_PER_SEC;
}


static void draw(void) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("draw() failed (error 0x%x)\n", err);
        exit(1);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -6.0f);
    glRotatef(tim()*55, 0.3f, 1.f, 1.3f);
    glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_SHORT, fanIx);
    glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_SHORT, &(fanIx[8]));
}

int main(int argc, char** argv) {

    XSetWindowAttributes winAttribs;
    Display *display;
    Window win;
    Colormap colormap;
    XVisualInfo *pVisual;
    XEvent e;

    GLint width =256, height = 256;
    
    /* EGL variables */
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    EGLSurface m_eglSurface;
    EGLint attributeList[] = { EGL_RED_SIZE, 1, EGL_DEPTH_SIZE, 1, EGL_NONE };
    EGLint		aEGLAttributes[] = {
          		                    EGL_RED_SIZE, 8,
          		                    EGL_GREEN_SIZE, 8,
          		                    EGL_BLUE_SIZE, 8,
          		                    EGL_DEPTH_SIZE, 16,
          		                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
          		                    EGL_NONE
    };
    EGLint		aEGLContextAttributes[] = {
          		                    EGL_CONTEXT_CLIENT_VERSION, 1,
          		                    EGL_NONE
    };
    EGLConfig m_eglConfig[1];
    EGLint nConfigs;

    /* Open X Display */
    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        /* Environment variable DISPLAY is not set */
        printf("Display not set.\n");
        goto exit;
    }
    printf("Native Display created");

    XSynchronize(display, True);
    
    /* EGL initialise */
    m_eglDisplay = eglGetDisplay(display);
    eglInitialize(m_eglDisplay, NULL, NULL);
    eglChooseConfig(m_eglDisplay, aEGLAttributes, m_eglConfig, 1, &nConfigs);

    /* Create X window */
    win = CreateXWindow(
            "OpenGL ES 1.1 Example on a Linux Desktop",
            width,
            height,
            display,
            m_eglDisplay,
            m_eglConfig[0],
            &colormap,
            &pVisual);
    if (!win)
    {
        /* Failed to create X window */
        printf("Failed to create X window.\n");
        goto exit;
    }
    printf("Created native window.\n");

    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig[0], win, 0);
    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig[0],
                                    EGL_NO_CONTEXT, aEGLContextAttributes);
    printf("EGLContext = %p\n", m_eglContext);
    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);

    /* Begin user code: */

    init();


    for (;;)
    {
        draw();
        eglSwapBuffers(m_eglDisplay, m_eglSurface);
    }
    /* End of user code. */

    /* EGL clean up */
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_eglDisplay, m_eglContext);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    eglTerminate(m_eglDisplay);

  exit: 
    /* X Windows clean up */
    XCloseDisplay(display);
    XDestroyWindow(display, win);
    XFreeColormap(display, colormap);
    XFree(pVisual);

    return 0;
}
