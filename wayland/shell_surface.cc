// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell_surface.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/surface.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace ozonewayland {

WaylandShellSurface::WaylandShellSurface(WaylandWindow* window)
    : surface_(NULL),
      shell_surface_(NULL)
{
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  if (!display)
      return;

  surface_ = new WaylandSurface();
  if (display->shell())
    shell_surface_ = wl_shell_get_shell_surface(
        display->shell(),
        surface_->wlSurface());

  if (shell_surface_)
  {
    static const wl_shell_surface_listener shell_surface_listener = {
      WaylandShellSurface::HandlePing,
      WaylandShellSurface::HandleConfigure,
      WaylandShellSurface::HandlePopupDone
    };

  wl_shell_surface_add_listener(shell_surface_, &shell_surface_listener, window);
  }
}

WaylandShellSurface::~WaylandShellSurface() {

  if (shell_surface_)
  {
    wl_shell_surface_destroy(shell_surface_);
    shell_surface_ = NULL;
  }

  if (surface_)
  {
    delete surface_;
    surface_ = NULL;
  }
}

void WaylandShellSurface::UpdateShellSurface(WaylandWindow::ShellType type,
                                             WaylandShellSurface* shell_parent,
                                             unsigned x,
                                             unsigned y) const
{
  switch (type) {
  case WaylandWindow::TOPLEVEL:
    wl_shell_surface_set_toplevel(shell_surface_);
    break;
  case WaylandWindow::POPUP: {
    WaylandDisplay* display = WaylandDisplay::GetInstance();
    WaylandInputDevice* input_device = display->PrimaryInput();
    wl_surface* surface = this->Surface()->wlSurface();
    wl_surface* parent_surface = shell_parent->Surface()->wlSurface();
    WaylandWindow* window =
        static_cast<WaylandWindow*>(wl_surface_get_user_data(surface));

    input_device->SetGrabWindow(window, 0);
    wl_shell_surface_set_popup(shell_surface_,
                               input_device->GetInputSeat(),
                               display->GetSerial(),
                               parent_surface,
                               x,
                               y,
                               0 /*flags*/);
    break;
  }
  case WaylandWindow::FULLSCREEN:
    wl_shell_surface_set_fullscreen(shell_surface_,
                                    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                    0,
                                    NULL);
    break;
  case WaylandWindow::CUSTOM:
      NOTREACHED() << "Unsupported shell type: " << type;
    break;
    default:
      break;
  }
}

void WaylandShellSurface::SetWindowTitle(const string16& title) {
  wl_shell_surface_set_title(shell_surface_, UTF16ToUTF8(title).c_str());
}

void WaylandShellSurface::HandleConfigure(void *data,
                                          struct wl_shell_surface *shell_surface,
                                          uint32_t edges,
                                          int32_t width,
                                          int32_t height)
{
}

void WaylandShellSurface::HandlePopupDone(void *data,
                                          struct wl_shell_surface *shell_surface)
{
  WaylandInputDevice* input_device =
      WaylandDisplay::GetInstance()->PrimaryInput();
  input_device->SetGrabWindow(NULL, 0);

  // TODO(vignatti): dispatch a close window to Chromium
}

void WaylandShellSurface::HandlePing(void *data,
                                     struct wl_shell_surface *shell_surface,
                                     uint32_t serial)
{
  wl_shell_surface_pong(shell_surface, serial);
}

}  // namespace ozonewayland
