/****************************************************************************
**
** Copyright (C) 2015 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://www.qt.io
**
** This file is part of QtEnterprise Embedded.
**
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://www.qt.io
**
****************************************************************************/

#pragma once

#include <QDebug>
#include <QImage>
#include <QPixmap>
//#include <QtMultimedia/QVideoFrame>
#include <opencv2/imgproc/imgproc.hpp>

// CV_8UC3|4 -> CV_8UC3
inline void ensureC3(cv::Mat *mat) {
  Q_ASSERT(mat->type() == CV_8UC3 || mat->type() == CV_8UC4);
  if (mat->type() != CV_8UC3) {
    cv::Mat tmp;
    cvtColor(*mat, tmp, cv::COLOR_BGRA2BGR);
    *mat = tmp;
  }
}

inline QImage Mat2QImage(const cv::Mat &mat) {
  switch (mat.type()) {
    case CV_8UC1: {
      QVector<QRgb> ct;
      for (int i = 0; i < 256; ++i)
        ct.append(qRgb(i, i, i));
      QImage result(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_Indexed8);
      result.bits();
      result.setColorTable(ct);
      return result.copy();
    }
    case CV_8UC3: {
      cv::Mat tmp;
      cvtColor(mat, tmp, cv::COLOR_BGR2BGRA);
      return Mat2QImage(tmp);
    }
    case CV_8UC4: {
      QImage result(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_RGB32);
      return result.rgbSwapped();
    }
    default:
      qWarning("Unhandled Mat format %d", mat.type());
      return QImage();
  }
}
/*
// YUV QVideoFrame -> CV_8UC3
inline cv::Mat yuvFrameToMat8(const QVideoFrame &frame) {
  // Q_ASSERT(frame.handleType() == QAbstractVideoBuffer::NoHandle &&
  // frame.isReadable());
  Q_ASSERT(frame.pixelFormat() == QVideoFrame::Format_YUV420P || frame.pixelFormat() == QVideoFrame::Format_NV12);

  const cv::Mat tmp(frame.height() + frame.height() / 2, frame.width(), CV_8UC1, const_cast<uchar *>(frame.bits()));
  cv::Mat result(frame.height(), frame.width(), CV_8UC3);
  cvtColor(tmp, result,
           frame.pixelFormat() == QVideoFrame::Format_YUV420P ? cv::COLOR_YUV2BGR_NV12 : cv::COLOR_YUV2BGR_NV12);
  return result;
}
*/
inline cv::Mat QImage2Mat(QImage const &src, bool bClone = true) {
  switch (src.format()) {
    // 8-bit, 4 channel
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied: {
      cv::Mat mat(src.height(), src.width(), CV_8UC4, const_cast<uchar *>(src.bits()),
                  static_cast<size_t>(src.bytesPerLine()));

      return (bClone ? mat.clone() : mat);
    }

    // 8-bit, 3 channel
    case QImage::Format_RGB32: {
      if (!bClone) {
        qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
      }

      cv::Mat mat(src.height(), src.width(), CV_8UC4, const_cast<uchar *>(src.bits()),
                  static_cast<size_t>(src.bytesPerLine()));

      cv::Mat matNoAlpha;

      cv::cvtColor(mat, matNoAlpha, cv::COLOR_BGRA2BGR); // drop the all-white alpha channel

      return matNoAlpha;
    }

    // 8-bit, 3 channel
    case QImage::Format_RGB888: {
      if (!bClone) {
        qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
      }

      QImage swapped = src.rgbSwapped();

      return cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar *>(swapped.bits()),
                     static_cast<size_t>(swapped.bytesPerLine()))
        .clone();
    }

    // 8-bit, 1 channel
    case QImage::Format_Indexed8: {
      cv::Mat mat(src.height(), src.width(), CV_8UC1, const_cast<uchar *>(src.bits()),
                  static_cast<size_t>(src.bytesPerLine()));

      return (bClone ? mat.clone() : mat);
    }

    default:
      qWarning() << "QImageToCvMat() - QImage format not handled in switch:" << src.format();
      break;
  }
  return cv::Mat();
}

inline cv::Mat QPixmap2Mat(const QPixmap &in, bool bclone) { return QImage2Mat(in.toImage(), bclone); }
