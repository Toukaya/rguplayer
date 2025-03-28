// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/config/core_config.h"

#include "base/debug/logging.h"
#include "base/exceptions/exception.h"
#include "inih/INIReader.h"
#include "rapidxml/rapidxml.hpp"

#include "SDL_messagebox.h"

namespace content {

namespace {

std::map<std::string, CoreConfigure::ANGLEBackend> kRendererMapping = {
    {"d3d11", CoreConfigure::ANGLEBackend::kD3D11},
    {"vulkan", CoreConfigure::ANGLEBackend::kVulkan},
    {"software", CoreConfigure::ANGLEBackend::kSoftware},
};

void ReplaceStringWidth(std::string& str, char before, char after) {
  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == before)
      str[i] = after;
}

char* IniStreamReader(char* str, int num, void* stream) {
  SDL_IOStream* io = static_cast<SDL_IOStream*>(stream);

  memset(str, 0, num);
  char c;
  int i = 0;

  while (i < num - 1 && SDL_ReadIO(io, &c, 1)) {
    str[i++] = c;
    if (c == '\n')
      break;
  }

  str[i] = '\0';
  return i ? str : nullptr;
}

}  // namespace

void CoreConfigure::LoadCommandLine(int argc, char** argv) {
  for (int i = 0; i < argc; ++i)
    args_.push_back(argv[i]);

  game_debug_ = false;
  game_battle_test_ = false;
  angle_renderer_ = ANGLEBackend::kDisable;

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "test" || std::string(argv[i]) == "debug")
      game_debug_ = true;
    if (std::string(argv[i]) == "btest")
      game_battle_test_ = true;
    if (std::string(argv[i]) == "use_angle" && argc != i + 1) {
      auto it = kRendererMapping.find(argv[i + 1]);
      if (it != kRendererMapping.end())
        angle_renderer_ = it->second;
    }
  }

  if (game_debug_)
    LOG(INFO) << "[App] Running debug test.";
  if (game_battle_test_)
    LOG(INFO) << "[App] Running battle test.";
}

bool CoreConfigure::LoadConfigure(SDL_IOStream* filestream,
                                  const std::string& app) {
  /* Parse configure */
  if (!filestream) {
    std::string str = "Failed to load configure file.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Kernel", str.c_str(),
                             nullptr);
    return false;
  }

  INIReader reader(filestream, IniStreamReader);
  SDL_CloseIO(filestream);
  if (reader.ParseError()) {
    std::string str = "Error when parse configure.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Kernel", str.c_str(),
                             nullptr);
    return false;
  }

  /* RGSS config part */
  game_rtp_ = reader.Get("Game", "RTP", std::string());
  game_title_ = reader.Get("Game", "Title", "RGU Default Widget");
  game_scripts_ = reader.Get("Game", "Scripts", std::string());
  ReplaceStringWidth(game_scripts_, '\\', '/');

  /* Core config */
  disable_menu_ = reader.GetBoolean("Kernel", "DisableMenu", false);
  disable_reset_ = reader.GetBoolean("Kernel", "DisableReset", false);
  async_renderer_ = reader.GetBoolean("Kernel", "AsyncRenderer", true);
  disable_audio_ = reader.GetBoolean("Kernel", "DisableAudio", false);
  rgss_version_ = (RGSSVersion)reader.GetInteger("Kernel", "RGSSVerison", 0);
  if (rgss_version_ == RGSSVersion::Null) {
    if (!game_scripts_.empty()) {
      rgss_version_ = RGSSVersion::RGSS1;

      const char* p = &game_scripts_[game_scripts_.size()];
      const char* head = &game_scripts_[0];

      while (--p != head)
        if (*p == '.')
          break;

      if (!strcmp(p, ".rvdata"))
        rgss_version_ = RGSSVersion::RGSS2;
      else if (!strcmp(p, ".rvdata2"))
        rgss_version_ = RGSSVersion::RGSS3;
    }
  }

  /* Renderer config */
  if (angle_renderer_ == ANGLEBackend::kDisable)
    angle_renderer_ =
        (ANGLEBackend)reader.GetInteger("Renderer", "UseANGLE", 0);
  renderer_debug_output_ = reader.GetBoolean("Renderer", "DebugOutput", false);
  initial_resolution_.x =
      reader.GetInteger("Renderer", "ScreenWidth",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 544 : 640);
  initial_resolution_.y =
      reader.GetInteger("Renderer", "ScreenHeight",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 416 : 480);
  window_size_.x =
      reader.GetInteger("Renderer", "WindowWidth", initial_resolution_.x);
  window_size_.y =
      reader.GetInteger("Renderer", "WindowHeight", initial_resolution_.y);
  allow_frame_skip_ = reader.GetBoolean("Renderer", "AllowFrameSkip", false);
  smooth_scale_ = reader.GetBoolean("Renderer", "SmoothScale", true);
  keep_ratio_ = reader.GetBoolean("Renderer", "KeepRatio", true);
  fullscreen_ = reader.GetBoolean("Renderer", "Fullscreen", false);
  background_running_ =
      reader.GetBoolean("Renderer", "BackgroundRunning", true);

  /* Filesystem */
  int size = reader.GetInteger("Filesystem", "LoadPathListSize", 0);
  for (int i = 0; i < size; ++i)
    load_paths_.push_back(reader.Get(
        "Filesystem", "LoadPath" + std::to_string(i + 1), std::string()));
  default_font_path_ =
      reader.Get("Filesystem", "DefaultFontPath", "Fonts/Default.ttf");
  i18n_xml_path_ = reader.Get("Filesystem", "XMLPath", app + ".xml");

  /* Platform */
  disable_ime_ = reader.GetBoolean("Platform", "DisableIME", false);

  return true;
}

void CoreConfigure::Loadi18nXML(filesystem::Filesystem* io) {
  SDL_IOStream* ops = nullptr;
  try {
    ops = io->OpenReadRaw(i18n_xml_path_);
  } catch (base::Exception& e) {
    LOG(INFO) << "[Config] Disable i18n XML Configure file.";
    return;
  }

  rapidxml::xml_document<> doc;

  int64_t fsize = SDL_GetIOSize(ops);
  char* buf = static_cast<char*>(SDL_malloc(fsize + 1));
  buf[fsize] = '\0';
  SDL_ReadIO(ops, buf, fsize);
  doc.parse<rapidxml::parse_default>(buf);

  rapidxml::xml_node<>* root = doc.first_node("translationbundle");
  if (root) {
    if (rapidxml::xml_attribute<>* lang = root->first_attribute("lang"))
      LOG(INFO) << "[Locate] Engine language: " << lang->value();

    for (rapidxml::xml_node<>* iter = root->first_node("translation"); iter;
         iter = iter->next_sibling("translation")) {
      rapidxml::xml_attribute<>* attr = iter->first_attribute("id");
      if (attr) {
        // Insert i18n translation
        i18n_translation_.emplace(std::stoi(attr->value()),
                                  std::string(iter->value()));
      }
    }
  }

  SDL_free(buf);
}

std::string CoreConfigure::GetI18NString(int ids,
                                         const std::string& default_value) {
  auto iter = i18n_translation_.find(ids);
  if (iter != i18n_translation_.end())
    return iter->second;
  return default_value;
}

}  // namespace content
