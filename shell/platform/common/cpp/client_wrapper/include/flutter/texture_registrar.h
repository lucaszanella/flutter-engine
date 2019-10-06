// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_TEXTURE_REGISTRAR_H_

#include <flutter_texture_registrar.h>

#include <stdint.h>
#include <memory>

namespace flutter {

// An external texture interface declaration.
class Texture {
 public:
  virtual ~Texture() {}
  // This is in response to the texture copy request interface, providing the
  // |height| and |width| parameters of bounds.
  // In some cases, we need to scale the texture to the bounds size to reduce
  // memory usage.
  virtual const PixelBuffer* CopyPixelBuffer(size_t width, size_t height) = 0;
};

// An external texture render interface declaration.
class TextureRenderer {
 public:
  virtual ~TextureRenderer() {}
  // This function renders to the shell's texture
  virtual void renderToTexture(size_t width, size_t height) = 0;
};


class TextureRegistrar {
 public:
  virtual ~TextureRegistrar() {}

  /**
   * Registers a |texture| object and return textureId.
   */
  virtual int64_t RegisterTexture(Texture* texture) = 0;

  /**
   * Mark a texture buffer is ready.
   */
  virtual void MarkTextureFrameAvailable(int64_t texture_id) = 0;

  /**
   * Unregisters an existing Texture object.
   */
  virtual void UnregisterTexture(int64_t texture_id) = 0;
  
};

class TextureRendererRegistrar {
 public:
  virtual ~TextureRendererRegistrar() {}


  virtual int64_t RegisterTextureRenderer(TextureRenderer* texture_renderer) = 0;

  /**
   * Mark a texture buffer is ready.
   */
  virtual void MarkTextureFrameAvailable(int64_t texture_id) = 0;

  /**
   * Unregisters an existing Texture object.
   */  
  virtual void UnregisterTextureRenderer(int64_t texture_id) = 0;

};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_TEXTURE_REGISTRAR_H_
