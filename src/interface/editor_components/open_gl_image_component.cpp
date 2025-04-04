#include "open_gl_image_component.h"

OpenGlImageComponent::OpenGlImageComponent(String name) : OpenGlComponent(name), component_(nullptr),
                                                          active_(true), static_image_(false),
                                                          paint_entire_component_(true) {
  image_.setTopLeft(-1.0f, 1.0f);
  image_.setTopRight(1.0f, 1.0f);
  image_.setBottomLeft(-1.0f, -1.0f);
  image_.setBottomRight(1.0f, -1.0f);
  image_.setColor(Colours::white);

  if (name == "")
    setInterceptsMouseClicks(false, false);
}

void OpenGlImageComponent::redrawImage(bool force) {
  if (!active_)
    return;

  Component* component = component_ ? component_ : this;

  int pixel_scale = Desktop::getInstance().getDisplays().findDisplayForPoint(getScreenPosition()).scale;
  int width = component->getWidth() * pixel_scale;
  int height = component->getHeight() * pixel_scale;
  if (width <= 0 || height <= 0)
    return;

  bool new_image = draw_image_ == nullptr || draw_image_->getWidth() != width || draw_image_->getHeight() != height;
  if (!new_image && (static_image_ || !force))
    return;

  image_.lock();

  if (new_image)
    draw_image_ = std::make_unique<Image>(Image::ARGB, width, height, false);

  draw_image_->clear(Rectangle<int>(0, 0, width, height));
  Graphics g(*draw_image_);
  g.addTransform(AffineTransform::scale(pixel_scale));
  paintToImage(g);
  image_.setImage(draw_image_.get());

  // Adjust texture coordinates to accommodate power-of-two sizing if needed.
  float gl_width = vital::utils::nextPowerOfTwo(width);
  float gl_height = vital::utils::nextPowerOfTwo(height);
  float width_ratio = gl_width / width;
  float height_ratio = gl_height / height;

  float right = -1.0f + 2.0f * width_ratio;
  float bottom = 1.0f - 2.0f * height_ratio;
  image_.setTopRight(right, 1.0f);
  image_.setBottomLeft(-1.0f, bottom);
  image_.setBottomRight(right, bottom);
  image_.unlock();
}

void OpenGlImageComponent::init(OpenGlWrapper& open_gl) {
  image_.init(open_gl);
}

void OpenGlImageComponent::render(OpenGlWrapper& open_gl, bool animate) {
  Component* component = component_ ? component_ : this;
  if (!active_ || !setViewPort(component, open_gl) || !component->isVisible())
    return;

  image_.drawImage(open_gl);
}

void OpenGlImageComponent::destroy(OpenGlWrapper& open_gl) {
  image_.destroy(open_gl);
}
