//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/SVGIconEngine.h"

#include <QPainter>
#include <QSvgRenderer>

namespace smtk
{
namespace extension
{

SVGIconEngine::SVGIconEngine(const std::string& iconBuffer)
{
  data = QByteArray::fromStdString(iconBuffer);
}

void SVGIconEngine::paint(
  QPainter* painter,
  const QRect& rect,
  QIcon::Mode mode,
  QIcon::State /*state*/)
{
  painter->save();
  if (mode == QIcon::Disabled)
  {
    // Custom icon engines do not receive Qt's synthesized disabled treatment.
    // Fade the SVG so disabled toolbar actions remain distinct on both light
    // and dark palettes.
    painter->setOpacity(painter->opacity() * 0.4);
  }
  QSvgRenderer renderer(data);
  renderer.render(painter, rect);
  painter->restore();
}

QIconEngine* SVGIconEngine::clone() const
{
  return new SVGIconEngine(*this);
}

QPixmap SVGIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
  // This function is necessary to create an EMPTY pixmap. It's called always
  // before paint()

  QImage img(size, QImage::Format_ARGB32);
  img.fill(qRgba(0, 0, 0, 0));
  QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
  {
    QPainter painter(&pix);
    QRect r(QPoint(0.0, 0.0), size);
    this->paint(&painter, r, mode, state);
  }
  return pix;
}
} // namespace extension
} // namespace smtk
