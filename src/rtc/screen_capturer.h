/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef SCREEN_CAPTURER_H_
#define SCREEN_CAPTURER_H_

#include <memory>
#include <thread>
#include <vector>

#include "api/scoped_refptr.h"
#include "api/video/i420_buffer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/video_capture/video_capture.h"

#include "rtc/scalable_track_source.h"
#include "rtc_base/ref_counted_object.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/video_common.h"

class ScreenCapturer : public ScalableVideoTrackSource,
                       public rtc::VideoSinkInterface<webrtc::VideoFrame>,
                       public webrtc::DesktopCapturer::Callback {
 public:
  static rtc::scoped_refptr<ScreenCapturer> Create();
  ScreenCapturer();
  virtual ~ScreenCapturer();

 private:
  bool Init();
  void Destroy();
  void CaptureThread();

  // rtc::VideoSinkInterface interface.
  void OnFrame(const webrtc::VideoFrame& frame) override;
  // overide webrtc::DesktopCapturer::Callback
  virtual void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                               std::unique_ptr<webrtc::DesktopFrame> frame);

  std::thread capturethread;
  std::unique_ptr<webrtc::DesktopCapturer> capturer;
  bool isrunning;
};

#endif  // SCREEN_CAPTURER_H_