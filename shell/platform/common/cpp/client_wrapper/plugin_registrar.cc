// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/plugin_registrar.h"

#include <iostream>
#include <map>

#include "include/flutter/engine_method_result.h"
#include "include/flutter/method_channel.h"

namespace flutter {

namespace {

// Passes |message| to |user_data|, which must be a BinaryMessageHandler, along
// with a BinaryReply that will send a response on |message|'s response handle.
//
// This serves as an adaptor between the function-pointer-based message callback
// interface provided by the C API and the std::function-based message handler
// interface of BinaryMessenger.
void ForwardToHandler(FlutterDesktopMessengerRef messenger,
                      const FlutterDesktopMessage* message,
                      void* user_data) {
  auto* response_handle = message->response_handle;
  BinaryReply reply_handler = [messenger, response_handle](
                                  const uint8_t* reply,
                                  const size_t reply_size) mutable {
    if (!response_handle) {
      std::cerr << "Error: Response can be set only once. Ignoring "
                   "duplicate response."
                << std::endl;
      return;
    }
    FlutterDesktopMessengerSendResponse(messenger, response_handle, reply,
                                        reply_size);
    // The engine frees the response handle once
    // FlutterDesktopSendMessageResponse is called.
    response_handle = nullptr;
  };

  const BinaryMessageHandler& message_handler =
      *static_cast<BinaryMessageHandler*>(user_data);

  message_handler(message->message, message->message_size,
                  std::move(reply_handler));
}

}  // namespace

// Wrapper around a FlutterDesktopMessengerRef that implements the
// BinaryMessenger API.
class BinaryMessengerImpl : public BinaryMessenger {
 public:
  explicit BinaryMessengerImpl(FlutterDesktopMessengerRef core_messenger)
      : messenger_(core_messenger) {}

  virtual ~BinaryMessengerImpl() = default;

  // Prevent copying.
  BinaryMessengerImpl(BinaryMessengerImpl const&) = delete;
  BinaryMessengerImpl& operator=(BinaryMessengerImpl const&) = delete;

  // |flutter::BinaryMessenger|
  void Send(const std::string& channel,
            const uint8_t* message,
            const size_t message_size) const override;

  // |flutter::BinaryMessenger|
  void Send(const std::string& channel,
            const uint8_t* message,
            const size_t message_size,
            BinaryReply reply) const override;

  // |flutter::BinaryMessenger|
  void SetMessageHandler(const std::string& channel,
                         BinaryMessageHandler handler) override;

 private:
  // Handle for interacting with the C API.
  FlutterDesktopMessengerRef messenger_;

  // A map from channel names to the BinaryMessageHandler that should be called
  // for incoming messages on that channel.
  std::map<std::string, BinaryMessageHandler> handlers_;
};

void BinaryMessengerImpl::Send(const std::string& channel,
                               const uint8_t* message,
                               const size_t message_size) const {
  FlutterDesktopMessengerSend(messenger_, channel.c_str(), message,
                              message_size);
}

void BinaryMessengerImpl::Send(const std::string& channel,
                               const uint8_t* message,
                               const size_t message_size,
                               BinaryReply reply) const {
  if (reply == nullptr) {
    FlutterDesktopMessengerSend(messenger_, channel.c_str(), message,
                                message_size);
    return;
  }
  struct Captures {
    BinaryReply reply;
  };
  auto captures = new Captures();
  captures->reply = reply;

  auto message_reply = [](const uint8_t* data, size_t data_size,
                          void* user_data) {
    auto captures = reinterpret_cast<Captures*>(user_data);
    captures->reply(data, data_size);
    delete captures;
  };
  bool result = FlutterDesktopMessengerSendWithReply(
      messenger_, channel.c_str(), message, message_size, message_reply,
      captures);
  if (!result) {
    delete captures;
  }
}

void BinaryMessengerImpl::SetMessageHandler(const std::string& channel,
                                            BinaryMessageHandler handler) {
  if (!handler) {
    handlers_.erase(channel);
    FlutterDesktopMessengerSetCallback(messenger_, channel.c_str(), nullptr,
                                       nullptr);
    return;
  }
  // Save the handler, to keep it alive.
  handlers_[channel] = std::move(handler);
  BinaryMessageHandler* message_handler = &handlers_[channel];
  // Set an adaptor callback that will invoke the handler.
  FlutterDesktopMessengerSetCallback(messenger_, channel.c_str(),
                                     ForwardToHandler, message_handler);
}

// Wrapper around a FlutterDesktopTextureRegistrarRef that implements the
// TextureRegistrar API.
class TextureRegistrarImpl : public TextureRegistrar {
 public:
  explicit TextureRegistrarImpl(
      FlutterDesktopTextureRegistrarRef texture_registrar_ref)
      : texture_registrar_ref_(texture_registrar_ref) {}

  virtual ~TextureRegistrarImpl() = default;

  // Prevent copying.
  TextureRegistrarImpl(TextureRegistrarImpl const&) = delete;
  TextureRegistrarImpl& operator=(TextureRegistrarImpl const&) = delete;

  virtual int64_t RegisterTexture(Texture* texture) override {
    int64_t texture_id = 0;
    if (texture->renderType == Texture::RenderType::CopyPixelBuffer) {
      std::cout << "Texture::RenderType::CopyPixelBuffer" << std::endl;
      FlutterTexutreCallback callback =
          [](size_t width, size_t height,
             void* user_data) -> const PixelBuffer* {
        return ((Texture*)user_data)->CopyPixelBuffer(width, height);
      };
      texture_id = FlutterDesktopRegisterExternalTexture(
          texture_registrar_ref_, callback, texture);
    } else if (texture->renderType == Texture::RenderType::RenderToTexture) {
      std::cout << "Texture::RenderType::RenderToTexture" << std::endl;
      FlutterTexutreRendererCallback callback =
          [](size_t width, size_t height, int64_t texture_id,
             void* user_data)  {
                std::cout << "texturerenderer callback def, userdata: " << user_data << std::endl;
        ((Texture*)user_data)->RenderToTexture(width, height, texture_id);
      };
      std::cout << "texture passed is " << texture << std::endl;
      texture_id = FlutterDesktopRegisterExternalTextureRenderer(
          texture_registrar_ref_, callback, texture);
    } else {
      std::cout << "NO TEXTURE TYPE!!!" << std::endl;
    }
    return texture_id;
  }

  virtual void MarkTextureFrameAvailable(int64_t texture_id) override {
    FlutterDesktopMarkExternalTextureFrameAvailable(texture_registrar_ref_,
                                                    texture_id);
  }

  virtual void UnregisterTexture(int64_t texture_id) override {
    FlutterDesktopUnregisterExternalTexture(texture_registrar_ref_, texture_id);
  }

 private:
  // Handle for interacting with the C API.
  FlutterDesktopTextureRegistrarRef texture_registrar_ref_;
};

// PluginRegistrar:

PluginRegistrar::PluginRegistrar(FlutterDesktopPluginRegistrarRef registrar)
    : registrar_(registrar) {
  auto core_messenger = FlutterDesktopRegistrarGetMessenger(registrar_);
  messenger_ = std::make_unique<BinaryMessengerImpl>(core_messenger);
  auto texture_registrar = FlutterDesktopGetTextureRegistrar(registrar_);
  textures_ = std::make_unique<TextureRegistrarImpl>(texture_registrar);
}

PluginRegistrar::~PluginRegistrar() {}

void PluginRegistrar::AddPlugin(std::unique_ptr<Plugin> plugin) {
  plugins_.insert(std::move(plugin));
}

void PluginRegistrar::EnableInputBlockingForChannel(
    const std::string& channel) {
  FlutterDesktopRegistrarEnableInputBlocking(registrar_, channel.c_str());
}

}  // namespace flutter
