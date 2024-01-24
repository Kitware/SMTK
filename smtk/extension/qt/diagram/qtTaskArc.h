//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskArc_h
#define smtk_extension_qtTaskArc_h

#include "smtk/extension/qt/diagram/qtObjectArc.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>

namespace smtk
{
namespace task
{
class Adaptor;
class Task;
} // namespace task

namespace extension
{

class qtTaskEditor;
class qtDiagramScene;
class qtBaseTaskNode;

/**\brief A graphics item that represents a task dependency or adaptor.
  *
  * These arcs are always drawn as curves normal to the rounded rectangle
  * used by default to render the task and have an arrow indicating the
  * direction of information propagation.
  */
class SMTKQTEXT_EXPORT qtTaskArc : public qtObjectArc
{
  Q_OBJECT

public:
  smtkSuperclassMacro(qtObjectArc);
  smtkTypeMacro(smtk::extension::qtTaskArc);

  /// Construct a dependency arc (of type "task dependency").
  qtTaskArc(
    qtDiagramGenerator* generator,
    qtBaseTaskNode* predecessor,
    qtBaseTaskNode* successor,
    QGraphicsItem* parent = nullptr);

  /// Construct an adaptor arc (whose type is the \a adaptor's typeName()).
  qtTaskArc(
    qtDiagramGenerator* generator,
    qtBaseTaskNode* predecessor,
    qtBaseTaskNode* successor,
    smtk::task::Adaptor* adaptor,
    QGraphicsItem* parent = nullptr);

  ~qtTaskArc() override;

  /// Return the object as required by our parent class.
  smtk::resource::PersistentObject* object() const override;

  /// Return the adaptor (or null for "task dependency" arcs).
  smtk::task::Adaptor* adaptor() const { return m_adaptor; }

  /// Return the endpoint tasks connected by this arc.
  smtk::task::Task* predecessorTask() const;
  smtk::task::Task* successorTask() const;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  int updateArcPoints() override;

protected:
  smtk::task::Adaptor* m_adaptor{ nullptr }; // Only set when ArcType == Adaptor.
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskArc_h
