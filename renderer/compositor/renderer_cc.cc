// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/compositor/renderer_cc.h"

namespace renderer {

CCLayer::CCLayer(base::WeakPtr<ui::Widget> window, const SDL_GLContext& gl_ctx)
    : gl_sdl_ctx_(gl_ctx), window_(window), texture_max_size_(0) {
  context_ = base::MakeRefCounted<gpu::GLES2CommandContext>();
  context_->InitContext();

  /* Init common indices object */
  quad_indices_buffer_ = base::MakeRefCounted<QuadIndicesBuffer>(context_);

  /* Init renderer state */
  states.frame_buffer_ = std::make_unique<GLFrameBufferTarget>(context_);
  states.viewport_ = std::make_unique<GLViewport>(context_);
  states.scissor_region_ = std::make_unique<GLScissorRegion>(context_);
  states.scissor_test_ = std::make_unique<GLScissorTest>(context_);
  states.blend_mode_ = std::make_unique<GLBlendMode>(context_);

  /* Init caps */
  context_->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size_);

  /* Init shaders */
  shaders.drawable_shader = std::make_unique<gpu::DrawableShader>(context_);

  /* Init viewport and clear */
  Viewport()->Set(base::Rect(base::Vec2i(), window->GetSize()));
  GetContext()->glClearColor(0, 0, 0, 0);
  GetContext()->glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window->AsSDLWindow());
}

}  // namespace renderer
