//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramLegendEntry.h"

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QColor>
#include <QEvent>
#include <QLayout>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QWidget>

namespace smtk
{
namespace extension
{

qtDiagramLegendEntry::qtDiagramLegendEntry(
  smtk::string::Token entryGroup,
  smtk::string::Token entryType,
  qtDiagramGenerator* generator,
  smtk::string::Token entryLabel)
  : Superclass(generator)
  , m_generator(generator)
  , m_group(entryGroup)
  , m_type(entryType)
  , m_label(entryLabel)
{
}

qtDiagramLegendEntry::~qtDiagramLegendEntry() = default;

qtDiagramScene* qtDiagramLegendEntry::scene() const
{
  return m_generator ? m_generator->diagram()->diagramScene() : nullptr;
}

void qtDiagramLegendEntry::paint(
  QPainter* painter,
  const QStyleOptionViewItem& option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  (void)painter;
  const auto& cfg(*this->scene()->configuration());

  QColor color = cfg.colorFromToken(m_type);
  qreal width = cfg.arcWidth();
  color.setAlphaF(1.0); // 0.75 for unselected
  QPen pen;
  pen.setWidth(width);
  pen.setBrush(color);

  QPainterPath path;
  qreal yy = option.rect.center().y();
  path.moveTo(option.rect.left() + width / 2.0, yy);
  path.lineTo(option.rect.right() - width / 2.0, yy);

  painter->setPen(pen);
  painter->strokePath(path, pen);
  // painter->fillPath(m_arrowPath, pen.brush());
}

} // namespace extension
} // namespace smtk
