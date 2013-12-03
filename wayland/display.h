// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_H_
#define OZONE_WAYLAND_DISPLAY_H_

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include "base/basictypes.h"

#include <list>
#include <map>
#include <wayland-client.h>

namespace ozonewayland {

class WaylandInputDevice;
class WaylandScreen;
class WaylandWindow;
class OzoneDisplay;

typedef std::map<unsigned, WaylandWindow*> WindowMap;

// WaylandDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandDisplay {
 public:
  static WaylandDisplay* GetInstance() { return instance_; }
  // Returns a pointer to wl_display.
  wl_display* display() const { return display_; }

  wl_registry* registry() const { return registry_; }

  // Returns a list of the registered screens.
  std::list<WaylandScreen*> GetScreenList() const { return screen_list_; }
  WaylandScreen* PrimaryScreen() const { return primary_screen_ ; }

  wl_shell* shell() const { return shell_; }

  wl_shm* shm() const { return shm_; }
  wl_compositor* GetCompositor() const { return compositor_; }
  int GetDisplayFd() const { return wl_display_get_fd(display_); }

  const WindowMap& GetWindowList() const { return widget_map_; }

  // Creates a WaylandWindow backed by EGL Window and maps it to w. This can be
  // useful for callers to track a particular surface. By default the type of
  // surface(i.e. toplevel, menu) is none. One needs to explicitly call
  // WaylandWindow::SetShellAttributes to set this. The ownership of
  // WaylandWindow is not passed to the caller.
  WaylandWindow* CreateAcceleratedSurface(unsigned w);

  // Destroys WaylandWindow whose handle is w.
  void DestroyWindow(unsigned w);

 private:
  enum RegistrationType {
    RegisterAsNeeded, // Handles all the required registrations.
    RegisterOutputOnly // Only screen registration.
  };

  WaylandDisplay(RegistrationType type);
  virtual ~WaylandDisplay();
  void terminate();
  void SyncDisplay();
  // This handler resolves all server events used in initialization. It also
  // handles input device registration, screen registration.
  static void DisplayHandleGlobal(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);
  // This handler resolves only screen registration. In general you don't want
  // to use this but the one below.
  static void DisplayHandleOutputOnly(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);

  // WaylandDisplay manages the memory of all these pointers.
  wl_display* display_;
  wl_registry* registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  wl_shm* shm_;
  WaylandScreen* primary_screen_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  WindowMap widget_map_;
  static WaylandDisplay* instance_;

  friend class OzoneDisplay;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
