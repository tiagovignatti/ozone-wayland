// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ipc/display_channel_host.h"

#include "ozone/impl/ozone_display.h"
#include "base/bind.h"
#include "content/public/browser/browser_thread.h"

namespace ozonewayland {

// This should be same as defined in display_channel.
#define CHANNEL_ROUTE_ID -0x1

OzoneDisplayChannelHost::OzoneDisplayChannelHost()
    : channel_(NULL),
      router_id_(0)
{
  dispatcher_ = WaylandDispatcher::GetInstance();
}

OzoneDisplayChannelHost::~OzoneDisplayChannelHost()
{
  OzoneDisplay::GetInstance()->OnChannelHostDestroyed();
  while (!deferred_messages_.empty()) {
    delete deferred_messages_.front();
    deferred_messages_.pop();
  }
}

void OzoneDisplayChannelHost::EstablishChannel()
{
  if (router_id_ == CHANNEL_ROUTE_ID)
    return;

  content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
      base::Bind(base::IgnoreResult(&OzoneDisplayChannelHost::UpdateConnection),
          this));
}

void OzoneDisplayChannelHost::ChannelClosed()
{
  router_id_ = 0;
  channel_ = NULL;
}

void OzoneDisplayChannelHost::SendWidgetState(unsigned w,
                                              unsigned state,
                                              unsigned width,
                                              unsigned height)
{
  if (router_id_ == CHANNEL_ROUTE_ID)
    Send(new WaylandWindow_State(router_id_, w, state, width, height));
  else
    deferred_messages_.push(new WaylandWindow_State(router_id_,
                                                    w,
                                                    state,
                                                    width,
                                                    height));
}

void OzoneDisplayChannelHost::SendWidgetAttributes(unsigned widget,
                                                   unsigned parent,
                                                   unsigned x,
                                                   unsigned y,
                                                   unsigned type)
{
  if (router_id_ == CHANNEL_ROUTE_ID)
    Send(new WaylandWindow_Attributes(router_id_, widget, parent, x, y, type));
  else
    deferred_messages_.push(new WaylandWindow_Attributes(router_id_,
                                                         widget,
                                                         parent,
                                                         x,
                                                         y,
                                                         type));
}

void OzoneDisplayChannelHost::SendWidgetTitle(
    unsigned w, const string16& title) {
  if (router_id_ == CHANNEL_ROUTE_ID)
    Send(new WaylandWindow_Title(router_id_, w, title));
  else
    deferred_messages_.push(new WaylandWindow_Title(router_id_,
                                                    w,
                                                    title));
}

void OzoneDisplayChannelHost::OnChannelEstablished(unsigned route_id)
{
  router_id_ = route_id;
  Send(new WaylandMsg_DisplayChannelEstablished(route_id));
  while (!deferred_messages_.empty()) {
    deferred_messages_.front()->set_routing_id(router_id_);
    Send(deferred_messages_.front());
    deferred_messages_.pop();
  }
}

void OzoneDisplayChannelHost::OnMotionNotify(float x, float y)
{
  dispatcher_->MotionNotify(x, y);
}

void OzoneDisplayChannelHost::OnButtonNotify(unsigned handle,
                                             int state,
                                             int flags,
                                             float x,
                                             float y)
{
  dispatcher_->ButtonNotify(handle, state, flags, x, y);
}

void OzoneDisplayChannelHost::OnAxisNotify(float x,
                                           float y,
                                           float xoffset,
                                           float yoffset)
{
  dispatcher_->AxisNotify(x, y, xoffset, yoffset);
}

void OzoneDisplayChannelHost::OnPointerEnter(unsigned handle, float x, float y)
{
  dispatcher_->PointerEnter(handle, x, y);
}

void OzoneDisplayChannelHost::OnPointerLeave(unsigned handle, float x, float y)
{
  dispatcher_->PointerLeave(handle, x, y);
}

void OzoneDisplayChannelHost::OnKeyNotify(unsigned type,
                                          unsigned code,
                                          unsigned modifiers)
{
  dispatcher_->KeyNotify(type, code, modifiers);
}

void OzoneDisplayChannelHost::OnOutputSizeChanged(unsigned width,
                                                  unsigned height)
{
  OzoneDisplay::GetInstance()->OnOutputSizeChanged(width, height);
}

bool OzoneDisplayChannelHost::OnMessageReceived(const IPC::Message& message)
{
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) <<
      "Must handle messages that were dispatched to another thread!";

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneDisplayChannelHost, message)
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, OnMotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, OnButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, OnAxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, OnPointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, OnPointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, OnKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OnOutputSizeChanged)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void OzoneDisplayChannelHost::OnFilterAdded(IPC::Channel* channel)
{
  channel_ = channel;
}

void OzoneDisplayChannelHost::OnChannelClosing()
{
  channel_ = NULL;
  router_id_ = 0;
}

bool OzoneDisplayChannelHost::Send(IPC::Message* message)
{
  if (channel_) {
    if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
      content::BrowserThread::PostTask(
          content::BrowserThread::IO,
          FROM_HERE,
          base::Bind(base::IgnoreResult(&OzoneDisplayChannelHost::Send), this,
                     message));
      return true;
    }

    return channel_->Send(message);
  }

  delete message;
  return false;
}

bool OzoneDisplayChannelHost::UpdateConnection()
{
  content::GpuProcessHost* host = content::GpuProcessHost::Get(
      content::GpuProcessHost::GPU_PROCESS_KIND_SANDBOXED,
      content::CAUSE_FOR_GPU_LAUNCH_BROWSER_STARTUP);

  DCHECK(host);
  host->AddFilter(this);
  OnChannelEstablished(CHANNEL_ROUTE_ID);
}

}  // namespace ozonewayland
