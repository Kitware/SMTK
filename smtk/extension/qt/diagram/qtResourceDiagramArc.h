//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtResourceDiagramArc_h
#define smtk_extension_qtResourceDiagramArc_h

#include "smtk/extension/qt/diagram/qtBaseArc.h"

namespace smtk
{
namespace extension
{

/**\brief A graphics item that represents an arc between nodes.
  *
  * These arcs are created by qtResourceDiagram to connect any
  * nodes representing components and/or resources.
  */
class SMTKQTEXT_EXPORT qtResourceDiagramArc : public qtBaseArc
{
  Q_OBJECT

public:
  smtkSuperclassMacro(qtBaseArc);
  smtkTypeMacro(smtk::extension::qtResourceDiagramArc);

  qtResourceDiagramArc(
    qtDiagramGenerator* generator,
    qtBaseNode* predecessor,
    qtBaseNode* successor,
    ArcType arcType,
    QGraphicsItem* parent = nullptr);
  ~qtResourceDiagramArc() override;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  int updateArcPoints() override;

protected:
  /// Draw the arc. This method is overridden to vary opacity with arc length.
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  qreal m_opacity{ 1. };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtResourceDiagramArc_h
