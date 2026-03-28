#pragma once
#include "error.hpp"
#include <QImage>
#include <QRect>
#include <QString>
#include <QVector>

namespace dndemask {

struct MaskFile {
  QString original_filename;
  QImage image;
  QVector<QRect> boxes;

  util::ErrorOr<void> save(const QString &path) const;
  static util::ErrorOr<MaskFile> load(const QString &path);
};

} // namespace dndemask
