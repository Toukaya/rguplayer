// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_CONTEXT_GLES2_CONTEXT_H_
#define RENDERER_CONTEXT_GLES2_CONTEXT_H_

#include "base/memory/ref_counted.h"

// clang-format off
#include "GLES3/gl3.h"
#include "GLES3/gl32.h"
#include "GLES2/gl2ext.h"
// clang-format on

#include <string>

namespace renderer {

class GLES2Context;

extern GLES2Context GL;

class GLES2Context {
 public:
  GLES2Context() = default;
  ~GLES2Context() = default;

  GLES2Context(const GLES2Context&) = delete;
  GLES2Context& operator=(const GLES2Context&) = delete;

  // Create the GLESContext on current thread
  static void CreateForCurrentThread(void* gl_library_handle);

  // KHR debug output
  static void EnableDebugOutputForCurrentThread();

 public:
  // Import from autogen-commands
#include "renderer/context/gles2_command_buffer_header_autogen.h"

 private:
  void InitGLESContext(void* gl_library_handle);
  void EnableDebugOutput();
  void* GetGLProc(const std::string& fname);

  void* ogl_library_ = nullptr;
  std::string suffix_;
};

}  // namespace renderer

#endif  //! RENDERER_CONTEXT_GLES2_CONTEXT_H_
