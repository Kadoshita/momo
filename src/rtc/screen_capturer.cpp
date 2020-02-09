/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "screen_capturer.h"

#include <stdint.h>

#include <memory>

// #include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

ScreenCapturer::ScreenCapturer() {
  capturer = webrtc::DesktopCapturer::CreateWindowCapturer(
      webrtc::DesktopCaptureOptions::CreateDefault());

  if (capturer) {
    webrtc::DesktopCapturer::SourceList sourceList;
    if (capturer->GetSourceList(&sourceList)) {
      RTC_LOG(LS_ERROR) << "capture window title : " << sourceList[1].title;
      capturer->SelectSource(sourceList[1].id);
    }
  }
}
ScreenCapturer::~ScreenCapturer() {
  Destroy();
}

bool ScreenCapturer::Init() {
  isrunning = true;
  capturethread = std::thread(&ScreenCapturer::CaptureThread, this);
  capturer->Start(this);

  return true;
}
void ScreenCapturer::CaptureThread() {
  RTC_LOG(INFO) << "DeviceDesktopCapturer:Run start";
  while (isrunning) {
    capturer->CaptureFrame();
  }
  RTC_LOG(INFO) << "DeviceDesktopCapturer:Run exit";
}

rtc::scoped_refptr<ScreenCapturer> ScreenCapturer::Create() {
  rtc::scoped_refptr<ScreenCapturer> screen_capturer(
      new rtc::RefCountedObject<ScreenCapturer>());
  if (!screen_capturer->Init()) {
    RTC_LOG(LS_WARNING) << "Failed to create ScreenCapturer";
    return nullptr;
  }
  return screen_capturer;
}

void ScreenCapturer::Destroy() {
  isrunning = false;
  capturethread.join();
}

void ScreenCapturer::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  RTC_LOG(LS_INFO) << "ScreenCapturer:OnCaptureResult";

  if (result == webrtc::DesktopCapturer::Result::SUCCESS) {
    int width = frame->rect().width();
    int height = frame->rect().height();
    rtc::scoped_refptr<webrtc::I420Buffer> I420buffer =
        webrtc::I420Buffer::Create(width, height);

    const int conversionResult = libyuv::ConvertToI420(
        frame->data(), frame->stride() * webrtc::DesktopFrame::kBytesPerPixel,
        I420buffer->MutableDataY(), I420buffer->StrideY(),
        I420buffer->MutableDataU(), I420buffer->StrideU(),
        I420buffer->MutableDataV(), I420buffer->StrideV(), 0, 0, width, height,
        width, height, libyuv::kRotate0, ::libyuv::FOURCC_ARGB);

    if (conversionResult >= 0) {
      webrtc::VideoFrame videoFrame(I420buffer,
                                    webrtc::VideoRotation::kVideoRotation_0,
                                    rtc::TimeMicros());
      if ((height == 0) && (width == 0)) {
        ScreenCapturer::OnFrame(videoFrame);
      } else {
        if (height == 0) {
          height = (videoFrame.height() * width) / videoFrame.width();
        } else if (width == 0) {
          width = (videoFrame.width() * height) / videoFrame.height();
        }
        int stride_y = width;
        int stride_uv = (width + 1) / 2;
        rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer =
            webrtc::I420Buffer::Create(width, height, stride_y, stride_uv,
                                       stride_uv);
        scaled_buffer->ScaleFrom(*videoFrame.video_frame_buffer()->ToI420());
        webrtc::VideoFrame frame = webrtc::VideoFrame(
            scaled_buffer, webrtc::kVideoRotation_0, rtc::TimeMicros());

        ScreenCapturer::OnFrame(frame);
      }
    } else {
      RTC_LOG(LS_ERROR) << "ScreenCapturer:OnCaptureResult conversion error:"
                        << conversionResult;
    }
  } else {
    RTC_LOG(LS_ERROR) << "ScreenCapturer:OnCaptureResult capture error:"
                      << (int)result;
  }
}
void ScreenCapturer::OnFrame(const webrtc::VideoFrame& frame) {
  OnCapturedFrame(frame);
}