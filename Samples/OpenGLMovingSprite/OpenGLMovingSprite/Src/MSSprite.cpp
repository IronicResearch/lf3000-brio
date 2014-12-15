#include "MSSprite.h"
#include <DisplayMPI.h>

CImageIO MSSprite::image_io_ = CImageIO();

MSSprite::MSSprite(LeapFrog::Brio::CPath& path) :
path_(path)
{
    image_io_.GetInfo(path_, image_surf_);
    image_surf_.buffer = NULL;
    image_io_.Load(path_, image_surf_);
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    switch(image_surf_.format)
    {
        case kPixelFormatRGB888:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_surf_.width, image_surf_.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_surf_.buffer);
            break;
        // TODO: The format of the pngs we have is actually RGBA although they
        // present themselves as kPixelFormatARGB8888.  The SDK should have the
        // ability to distinguish between these types as the latter requires
        // byte shifting for each pixel to conform to what OpenGL requires where
        // as the former does not.
        case kPixelFormatARGB8888:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_surf_.width, image_surf_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_surf_.buffer);
            break;
    }
    delete[]image_surf_.buffer;

    // the images loaded into the sprites have a coordinate system that starts
    // in the top left corner of the image, hence we have to flip the texture
    // about the y axis.  This is why we are ordering the texture coordinates in
    // the following manner:
    //  0 *----* 1
    //    |    |
    //  2 *----* 3
    // the devices have origins in the following locations:
    // LeapPad2: holding in portrait mode, lower right corner
    //            positive x, along vertical right edge
    //            positive y, right to left along bottom edge
    //
    // LeapsterGS: lower left corner
    //             positive x along vertical left edge
    //             positive y, left to right along bottom edge
    texcoord_[0] = 0; texcoord_[1] = 1;
    texcoord_[2] = 1; texcoord_[3] = 1;
    texcoord_[4] = 0; texcoord_[5] = 0;
    texcoord_[6] = 1; texcoord_[7] = 0;
    frame_ = MSRect(MSPoint(0,0), MSSize(image_surf_.width, image_surf_.height));
    UpdateVertices();
}

void MSSprite::UpdateVertices(void)
{
    // updates the on screen vertices for rendering
    vertices_[0] = frame_.origin_.x_;
    vertices_[1] = frame_.origin_.y_;

    vertices_[2] = frame_.origin_.x_+frame_.size_.width_;
    vertices_[3] = frame_.origin_.y_;

    vertices_[4] = frame_.origin_.x_;
    vertices_[5] = frame_.origin_.y_+frame_.size_.height_;

    vertices_[6] = frame_.origin_.x_+frame_.size_.width_;
    vertices_[7] = frame_.origin_.y_+frame_.size_.height_;
}

void MSSprite::SetFrame(const MSRect& rect)
{
    frame_ = rect;
    UpdateVertices();
}

MSSize MSSprite::ImageSize(void)
{
    return MSSize(static_cast<float>(image_surf_.width),
                  static_cast<float>(image_surf_.height));
}

void MSSprite::Render(void)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, texture_);
    glVertexPointer(2, GL_FLOAT, 0, vertices_);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoord_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

}

// Animation Class
MSAnimation::MSAnimation(void) :
    looping_(false),
    current_sprite_(0)
{
}

MSAnimation::~MSAnimation(void)
{
    sprites_.clear();
}

MSSize MSAnimation::ImageSize(void)
{
    return (sprites_.size() > 0) ? sprites_[0].ImageSize() : MSSize();
}

void MSAnimation::AddSprite(MSSprite sprite)
{
    // the assumption is that all sprites are the same size
    // maybe we should do some error checking here to enforce that assumption
    sprites_.push_back(sprite);
}

void MSAnimation::SetTranslation(const MSVector& vec)
{
    translation_ = vec;
}

bool MSAnimation::OnScreen(void)
{
	// NOTE:
	// There is a very big assumption in this code...
	// This assumes that the transform matrix coming in to the animation's render method
	// is the identity matrix.  It's possible there already exists a translation in the
	// matrix that by itself could place the animation off the screen, or in combination
	// with the translation in the animation could place the animation off the screen.
	// A better practice would be to check the translation from the matrix and the translation
	// in this class to determine if the animation is on screen.
	static LeapFrog::Brio::CDisplayMPI display_mpi;
	static const LeapFrog::Brio::tDisplayScreenStats* screen_stats = display_mpi.GetScreenStats(LeapFrog::Brio::kOrientationPortrait);
	static GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	float dx = translation_.x_ + m[12]; // 12 is the index to the x translation value in the model view matrix
	float dy = translation_.y_ + m[13]; // 13 is the index to the y translation value in the model view matrix
	MSSize size = ImageSize();
	if (dx > (screen_stats->width-size.width_) ||
		dx < (-size.width_) ||
		dy > (screen_stats->height-size.height_) ||
		dy < (-size.height_))
	{
		return false;
	}
	return true;
}
bool MSAnimation::ShouldRender(void)
{
    return (current_sprite_ < sprites_.size() && OnScreen());
}

void MSAnimation::Render(void) {
    if (ShouldRender()) {
        // push a matrix on the opengl render stack.  This allows us to make
        // local modifications to the view matrix without disturbing any
        // rendering that occurs outside of this class after the Render
        // method has completed.
        glPushMatrix();

        // we are forcing a 2D world but in actuality the render buffer is a
        // 3d buffer, hence the hard coded 0.0 for the z-translation
        glTranslatef(translation_.x_, translation_.y_, 0.0);

        sprites_[current_sprite_].Render();

        // pop the view matrix to prevent the transformations we made in this
        // method from propagating beyond this method
        glPopMatrix();

        current_sprite_ = (++current_sprite_ >= sprites_.size() && looping_) ? 0 : current_sprite_;
    }
}
