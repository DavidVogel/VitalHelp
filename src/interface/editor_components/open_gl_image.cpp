#include "open_gl_image.h"

#include "open_gl_component.h"
#include "shaders.h"
#include "utils.h"

namespace {
  constexpr int kNumPositions = 16;        // Number of floats in the vertex data (4 vertices * 4 floats).
  constexpr int kNumTriangleIndices = 6;   // Number of indices for drawing two triangles forming a quad.
} // namespace

OpenGlImage::OpenGlImage() : dirty_(true), image_(nullptr), image_width_(0), image_height_(0),
                             additive_(false), use_alpha_(false), scissor_(false) {
  position_vertices_ = std::make_unique<float[]>(kNumPositions);
  float position_vertices[kNumPositions] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, -1.0f, 0.0f, 0.0f,
    0.1f, -1.0f, 1.0f, 0.0f,
    0.1f, 1.0f, 1.0f, 1.0f
  };
  memcpy(position_vertices_.get(), position_vertices, kNumPositions * sizeof(float));

  position_triangles_ = std::make_unique<int[]>(kNumTriangleIndices);
  int position_triangles[kNumTriangleIndices] = {
    0, 1, 2,
    2, 3, 0
  };
  memcpy(position_triangles_.get(), position_triangles, kNumTriangleIndices * sizeof(int));
}

OpenGlImage::~OpenGlImage() { }

void OpenGlImage::init(OpenGlWrapper& open_gl) {
  // Create and bind vertex buffer for position data
  open_gl.context.extensions.glGenBuffers(1, &vertex_buffer_);
  open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

  GLsizeiptr vert_size = static_cast<GLsizeiptr>(static_cast<size_t>(kNumPositions * sizeof(float)));
  open_gl.context.extensions.glBufferData(GL_ARRAY_BUFFER, vert_size,
                                         position_vertices_.get(), GL_STATIC_DRAW);

  // Create and bind element buffer for triangle indices
  open_gl.context.extensions.glGenBuffers(1, &triangle_buffer_);
  open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer_);

  GLsizeiptr tri_size = static_cast<GLsizeiptr>(static_cast<size_t>(kNumTriangleIndices * sizeof(float)));
  open_gl.context.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, tri_size,
                                          position_triangles_.get(), GL_STATIC_DRAW);

  // Use a tinted image shader
  image_shader_ = open_gl.shaders->getShaderProgram(Shaders::kImageVertex, Shaders::kTintedImageFragment);

  image_shader_->use();
  image_color_ = OpenGlComponent::getUniform(open_gl, *image_shader_, "color");
  image_position_ = OpenGlComponent::getAttribute(open_gl, *image_shader_, "position");
  texture_coordinates_ = OpenGlComponent::getAttribute(open_gl, *image_shader_, "tex_coord_in");
}

void OpenGlImage::drawImage(OpenGlWrapper& open_gl) {
  // Load image into texture if a new image was set
  mutex_.lock();
  if (image_) {
    texture_.loadImage(*image_);
    image_ = nullptr;
  }
  mutex_.unlock();

  glEnable(GL_BLEND);
  if (scissor_)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);

  // Set blending mode
  if (additive_)
    glBlendFunc(GL_ONE, GL_ONE);
  else if (use_alpha_)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  else
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Update vertex data if needed
  open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  GLsizeiptr vert_size = static_cast<GLsizeiptr>(static_cast<size_t>(kNumPositions * sizeof(float)));

  mutex_.lock();
  if (dirty_)
    open_gl.context.extensions.glBufferData(GL_ARRAY_BUFFER, vert_size, position_vertices_.get(), GL_STATIC_DRAW);
  dirty_ = false;

  open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer_);
  texture_.bind();
  open_gl.context.extensions.glActiveTexture(GL_TEXTURE0);
  mutex_.unlock();

  image_shader_->use();

  // Set uniform color
  image_color_->set(color_.getFloatRed(), color_.getFloatGreen(), color_.getFloatBlue(), color_.getFloatAlpha());

  // Enable vertex attributes
  open_gl.context.extensions.glVertexAttribPointer(image_position_->attributeID, 2, GL_FLOAT,
                                                   GL_FALSE, 4 * sizeof(float), nullptr);
  open_gl.context.extensions.glEnableVertexAttribArray(image_position_->attributeID);
  open_gl.context.extensions.glVertexAttribPointer(texture_coordinates_->attributeID, 2, GL_FLOAT,
                                                   GL_FALSE, 4 * sizeof(float),
                                                   (GLvoid*)(2 * sizeof(float)));
  open_gl.context.extensions.glEnableVertexAttribArray(texture_coordinates_->attributeID);

  // Draw the image quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

  // Disable vertex attributes
  open_gl.context.extensions.glDisableVertexAttribArray(image_position_->attributeID);
  open_gl.context.extensions.glDisableVertexAttribArray(texture_coordinates_->attributeID);
  texture_.unbind();

  open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
  open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisable(GL_BLEND);
  glDisable(GL_SCISSOR_TEST);
}

void OpenGlImage::destroy(OpenGlWrapper& open_gl) {
  texture_.release();

  image_shader_ = nullptr;
  image_color_ = nullptr;
  image_position_ = nullptr;
  texture_coordinates_ = nullptr;

  open_gl.context.extensions.glDeleteBuffers(1, &vertex_buffer_);
  open_gl.context.extensions.glDeleteBuffers(1, &triangle_buffer_);
}
