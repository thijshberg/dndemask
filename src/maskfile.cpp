#include "maskfile.hpp"
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QIODevice>

namespace dndemask {
using namespace dndemask::util;

// Format:
//   magic:   u8[8]  "DNDEMASK"
//   version: u32    1
//   source:  QString
//   boxes:   u32 count, then per box: i32 x, i32 y, i32 w, i32 h
//   image:   QByteArray (PNG encoded)

static constexpr char MAGIC[] = "DNDEMASK";

ErrorOr<void> MaskFile::save(const QString &path) const {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    return Error("Could not open file for writing: " + path.toStdString());

  QDataStream out(&file);
  out.setVersion(QDataStream::Qt_6_0);

  out.writeRawData(MAGIC, 8);
  out << quint32(1);
  out << original_filename;
  out << quint32(boxes.size());
  for (const QRect &r : boxes)
    out << qint32(r.x()) << qint32(r.y()) << qint32(r.width()) << qint32(r.height());

  QByteArray imageData;
  QBuffer buf(&imageData);
  buf.open(QIODevice::WriteOnly);
  image.save(&buf, "PNG");
  out << imageData;

  if (out.status() != QDataStream::Ok)
    return Error("Write error while saving maskfile");

  return {};
}

ErrorOr<MaskFile> MaskFile::load(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return Error("Could not open file for reading: " + path.toStdString());

  QDataStream in(&file);
  in.setVersion(QDataStream::Qt_6_0);

  char magic[8];
  if (in.readRawData(magic, 8) != 8 || memcmp(magic, MAGIC, 8) != 0)
    return Error("Not a valid maskfile");

  quint32 version;
  in >> version;
  if (version != 1)
    return Error("Unsupported maskfile version");

  MaskFile mf;
  in >> mf.original_filename;

  quint32 boxCount;
  in >> boxCount;
  mf.boxes.reserve(boxCount);
  for (quint32 i = 0; i < boxCount; ++i) {
    qint32 x, y, w, h;
    in >> x >> y >> w >> h;
    mf.boxes.append(QRect(x, y, w, h));
  }

  QByteArray imageData;
  in >> imageData;
  if (!mf.image.loadFromData(imageData, "PNG"))
    return Error("Could not decode image from maskfile");

  if (in.status() != QDataStream::Ok)
    return Error("Read error while loading maskfile");

  return mf;
}

} // namespace dndemask
