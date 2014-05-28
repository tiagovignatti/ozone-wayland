// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/ozone_platform_media_wayland.h"

#include "media/video/video_decode_accelerator.h"
#include "media/ozone/ozone_platform.h"
#include "ozone/media/vaapi_video_decode_accelerator.h"

namespace media {

namespace {

class OzonePlatformMediaWayland : public OzonePlatformMedia {
 public:
  OzonePlatformMediaWayland() {}

  virtual ~OzonePlatformMediaWayland() {}

  // OzonePlatformMedia:
  virtual media::VideoDecodeAccelerator* CreateVideoDecodeFactoryOzone(
      const base::Callback<bool(void)>& make_context_current) OVERRIDE {
    return new VaapiVideoDecodeAccelerator(make_context_current);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(OzonePlatformMediaWayland);
};

}  // namespace

OzonePlatformMedia* CreateOzonePlatformMediaWayland() { return new OzonePlatformMediaWayland; }

}  // namespace media
