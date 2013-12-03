// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_SURFACE_H_
#define OZONE_WAYLAND_SHELL_SURFACE_H_

#include "base/basictypes.h"
#include "ozone/wayland/window.h"

#include <wayland-client.h>

namespace ozonewayland {

class WaylandSurface;
class WaylandWindow;

class WaylandShellSurface {
 public:
  WaylandShellSurface(WaylandWindow* window);
  ~WaylandShellSurface();

  void UpdateShellSurface(WaylandWindow::ShellType type,
                          WaylandShellSurface* shell_parent,
                          unsigned x,
                          unsigned y) const;
  void SetWindowTitle(const string16& title);
  WaylandSurface* Surface() const { return surface_; }

  static void HandleConfigure(void *data,
                              struct wl_shell_surface *shell_surface,
                              uint32_t edges,
                              int32_t width,
                              int32_t height);
  static void HandlePopupDone(void *data,
                              struct wl_shell_surface *shell_surface);
  static void HandlePing(void *data,
                         struct wl_shell_surface *shell_surface,
                         uint32_t serial);

 private:
  WaylandSurface* surface_;
  wl_shell_surface* shell_surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandShellSurface);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_SURFACE_H_
