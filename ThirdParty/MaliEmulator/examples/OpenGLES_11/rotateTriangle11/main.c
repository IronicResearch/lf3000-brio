/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2007, 2009, 2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include <GLES/gl.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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

int main(void) {

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
    unsigned char mIndices[] = { 0, 1, 2 };
    signed short mVertices[] = {
                                -50, -29, 0,
                                50, -29, 0,
                                0,  58, 0
    };
    int bDone = 0;
    
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
    
    /* EGL init */
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
    
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig[0], win, 0);
    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig[0],
                                    EGL_NO_CONTEXT, aEGLContextAttributes);
    printf("EGLContext = %p\n", m_eglContext);
    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_SHORT, 0, mVertices);

    /* Set projection matrix so screen extends to (-160, -120) at bottom left
     * and to (160, 120) at top right, with -128..128 as Z buffer. */
    glMatrixMode(GL_PROJECTION);
    glOrthox(-160<<16, 160<<16, -120<<16, 120<<16, -128<<16, 128<<16);

    glMatrixMode(GL_MODELVIEW);

    glClearColorx(0x10000, 0x10000, 0, 0);
    glColor4x(0x10000, 0, 0, 0);

    /* Main event loop */
    while(!bDone)
    {
        /* Do Windows stuff: */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, mIndices);
        glRotatex(2<<16, 0, 0, 0x10000);
        eglSwapBuffers(m_eglDisplay, m_eglSurface);
        usleep(1000);
    }

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
