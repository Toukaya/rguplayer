// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_
#define RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_

#include <memory>

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"

namespace renderer {

class GLTexture : public base::RefCountedThreadSafe<GLTexture> {
 public:
  GLTexture(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~GLTexture();

  GLTexture(const GLTexture&) = delete;
  GLTexture& operator=(const GLTexture&) = delete;

  void SetSize(const base::Vec2i& size) { size_ = size; }
  base::Vec2i GetSize() { return size_; }

  void AllocEmpty();
  void BufferData(const void* data, GLenum format);
  void BufferData(const base::Vec4i& bounds, const void* data, GLenum format);

  void Activate(uint16_t tex_unit);
  GLuint GetTextureRaw() { return texture_; }
  void Bind();
  void Unbind();

  void SetTextureWrap(GLint mode);
  void SetTextureFilter(GLint mode);

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  base::Vec2i size_;
  GLuint texture_;
};

class GLFrameBuffer {
 public:
  GLFrameBuffer(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~GLFrameBuffer();

  GLFrameBuffer(const GLFrameBuffer&) = delete;
  GLFrameBuffer& operator=(const GLFrameBuffer&) = delete;

  void SetRenderTarget(scoped_refptr<GLTexture> target);
  scoped_refptr<GLTexture> GetRenderTarget() { return texture_; }

  void ReadPixels(const base::Vec4i& bounds, GLenum format, GLenum data_type,
                  void* pixels);

  void Bind();
  void Unbind();

  void Clear();
  void Clear(const base::Vec4& color);

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  scoped_refptr<GLTexture> texture_;
  GLuint frame_buffer_;
};

}  // namespace renderer

#endif  // RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_