#ifndef GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSSPRITE_H_
#define GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSSPRITE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file MSSprite.h
*
* The MSSprite file holds a collections of structs and classes used to enable
* simple rendering of sprites and animations.  These classes is meant to be a
* reference and not a complete implementation as some essential error checking
* is not in place and some (probably) required functionality for a full
* sprite class is not present.
**
* author: 	leapfrog
*  			alindblad - 11/8/12 - created initial class
*
******************************************************************************/
#include <ImageIO.h>
#include <GLES/gl.h>
#include <vector>

/**
 * MSPoint is a simple 2d point class with three different means of construction
 */
struct MSPoint
{
    float x_;
    float y_;
    MSPoint(float x, float y) : x_(x), y_(y) { }
    MSPoint(const MSPoint& point) :
        x_(point.x_), y_(point.y_) { }
    MSPoint(void) : x_(0), y_(0) { }
};

/**
 * MSVector is a typedef'ed MSPoint, functionally equivalent
 */
typedef MSPoint MSVector;

/*
 * MSSize is a simple size class containing a width and height.  Functionally
 * no different than MSPoint, just different variable names.
 */
struct MSSize
{
    float width_;
    float height_;
    MSSize(float width, float height) :
        width_(width), height_(height) { }
    MSSize(const MSSize& size) :
        width_(size.width_), height_(size.height_) { }
    MSSize(void) : width_(0), height_(0) { }
};

/*
 * MSRect is a basic rectangle class used to define the render frame for sprites.
 * It stores the origin, assumed to be the lower left corner and the size of the
 * rectangle.
 */
struct MSRect
{
    MSPoint origin_;
    MSSize size_;
    MSRect(MSPoint origin, MSSize size) :
        origin_(origin), size_(size) { }
    MSRect(const MSRect& rect) :
    	origin_(rect.origin_), size_(rect.size_) { }
    MSRect(void) { }
};

/*
 * MSSprite is a basic sprite class that loads an image from file and renders
 * the image to the screen using a 2D texture map in opengl.  The sprite allows
 * for on screen movement via a Translate and SetFrame method.
 */
class MSSprite
{
public:
    // each sprite requires a path for loading the image
    MSSprite(LeapFrog::Brio::CPath& path);

    void Render(void);

    /*
     * SetFrame moves the sprite to the coordinates specified in the MSRect
     * variable passed in.  This is not an accumulated method but rather it sets
     * the absolute position, and scale, of the image on the screen.
     */
    void SetFrame(const MSRect& rect);
    MSSize ImageSize(void);

protected:
    static CImageIO        image_io_;     ///< loads image data

    LeapFrog::Brio::CPath  path_;         ///< path for image to display
    tVideoSurf             image_surf_;   ///< keeps meta data of the image
    GLuint                 texture_;      ///< handle for opengl texture
    MSRect                 frame_;
    float                  texcoord_[8];  ///< Corresponding texture coordinates
    float                  vertices_[8];  ///< on screen coordinates for texture rendering

private:
    /*
     * Internal method used to update the vertices needed by opengl to render
     * the sprite to physical screen coordinates.
     */
    void UpdateVertices(void);
};

/*
 * MSAnimation is a basic animation class capable of playing a single animation
 * or a looping animation (infinite).  It also exposes two methods to move the
 * animation on the screen, SetTranslation and SetRotation
 *
 * The sprites are rendered in the order they are added to the animation
 */
class MSAnimation
{
public:
    MSAnimation(void);
    ~MSAnimation(void);

    void Render(void);

    void AddSprite(MSSprite sprite);
    void SetTranslation(const MSVector& vec);
    MSSize ImageSize(void);

    bool looping_;  ///< indicates if the animation should loop
protected:
    std::vector<MSSprite> sprites_;              ///< vector of sprites, animate in order
    unsigned int current_sprite_;                ///< index of sprite to render next
    MSVector     translation_;                   ///< 2d translation in x, y

private:
    /*
     * Checks to see if the animation as a sprite to render based on if the
     * animation is looping and where it is in the sequence of sprites
     */
    bool ShouldRender(void);

    /*
     * OnScreen
     *
     * Checks to see if the sprite is on the screen.  This is used
     * to determine if the current sprite of the animation should
     * be rendered.  If it's offscreen, do not render.
     */
    bool OnScreen(void);

};
#endif // GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSSPRITE_H_
