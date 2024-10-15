//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskPortArc_h
#define smtk_extension_qtTaskPortArc_h

#include "smtk/extension/qt/diagram/qtBaseArc.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>

namespace smtk
{
namespace task
{
class Task;
class Port;
} // namespace task

namespace extension
{

class qtTaskEditor;
class qtDiagramScene;
class qtBaseObjectNode;
class qtTaskPortNode;

/**\brief A graphics item that represents a task dependency or adaptor.
  *
  * These arcs are always drawn as curves normal to the rounded rectangle
  * used by default to render the task and have an arrow indicating the
  * direction of information propagation.
  */
class SMTKQTEXT_EXPORT qtTaskPortArc : public qtBaseArc
{
  Q_OBJECT

public:
  smtkSuperclassMacro(qtBaseArc);
  smtkTypeMacro(smtk::extension::qtTaskPortArc);

  /// Construct a dependency arc (of type "task dependency").
  qtTaskPortArc(
    qtDiagramGenerator* generator,
    qtBaseObjectNode* predecessor,
    qtTaskPortNode* successor,
    QGraphicsItem* parent = nullptr);

  ~qtTaskPortArc() override;

  /// Return the endpoint tasks connected by this arc.
  smtk::resource::PersistentObject* predecessorObject() const;
  smtk::task::Port* successorPort() const;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  int updateArcPoints() override;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskPortArc_h
