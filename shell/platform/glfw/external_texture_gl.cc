// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/external_texture_gl.h"

#include <glad/gl.h>

// glad.h must be included before glfw3.h.
#include <GLFW/glfw3.h>

namespace flutter {

// Just to not declare GLFWwindow and GLuint in the header.
struct ExternalTextureGLState {
  GLFWwindow* window;
  GLuint gl_texture;
};

ExternalTextureGL::ExternalTextureGL(FlutterTexutreCallback texture_callback,
                                     void* user_data)
    : state_(new ExternalTextureGLState()),
      texture_callback_(texture_callback),
      user_data_(user_data) {}

ExternalTextureGL::ExternalTextureGL(FlutterTexutreRendererCallback texture_renderer_callback,
                                     void* user_data)
    : state_(new ExternalTextureGLState()),
      texture_renderer_callback_(texture_renderer_callback),
      user_data_(user_data) {}

ExternalTextureGL::~ExternalTextureGL() {
  if (state_->gl_texture != 0)
    glDeleteTextures(1, &state_->gl_texture);
}

bool ExternalTextureGL::PopulateTextureWithIdentifier(
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  // Confirm that the current window context is available.
  if (!state_->window) {
    state_->window = glfwGetCurrentContext();
    if (state_->window) {
      glfwMakeContextCurrent(state_->window);
      // Load GL functions.
      gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    }
  }

  if (!state_->window)
    return false;

  if (state_->gl_texture == 0) {
    glGenTextures(1, &state_->gl_texture);
    glBindTexture(GL_TEXTURE_2D, state_->gl_texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  if (texture_callback_) {
     const PixelBuffer* pixel_buffer =
        texture_callback_(width, height, user_data_);
    if (!pixel_buffer || !pixel_buffer->buffer)
    return false;
  
  
    glBindTexture(GL_TEXTURE_2D, state_->gl_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_buffer->width,
                pixel_buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                pixel_buffer->buffer);
  } else if(texture_renderer_callback_) {
    texture_renderer_callback_(width, height, user_data_);
  }

  opengl_texture->target = GL_TEXTURE_2D;
  opengl_texture->name = state_->gl_texture;
  opengl_texture->format = GL_RGBA8;
  opengl_texture->destruction_callback = (VoidCallback) nullptr;
  opengl_texture->user_data = (void*)this;
  return true;
}

}  // namespace flutter
