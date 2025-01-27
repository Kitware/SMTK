//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtBaseNode_h
#define smtk_extension_qtBaseNode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsObject>
#include <QGraphicsScene>

class QAbstractItemModel;
class QGraphicsTextItem;
class QItemSelection;
class QTimer;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtDiagram;
class qtDiagramGenerator;
class qtDiagramScene;

/**\brief A Graphical Item that represents a node connected by arcs in a scene.
  *
  * Any object that the qtDiagramScene should be able to lay out as a graph
  * must inherit this class.
  * Note that grouping nodes do not necessarily have to be persistent objects,
  * so this class just assumes that each instance can provide a UUID along
  * with connections (of various edge types) to other qtBaseNode instances
  * via a qtBaseArc instance.
  */
class SMTKQTEXT_EXPORT qtBaseNode : public QGraphicsObject
{
  Q_OBJECT
  Q_PROPERTY(ContentStyle contentStyle READ contentStyle WRITE setContentStyle);

public:
  smtkSuperclassMacro(QGraphicsObject);
  smtkTypeMacroBase(smtk::extension::qtBaseNode);

  /// Determine how the node is presented to users.
  enum class ContentStyle : int
  {
    Minimal, //!< Only the node's title-bar is shown.
    Summary, //!< The node's title bar and a "mini" viewer should be shown.
    Details  //!< The node's full state and any contained view should be shown.
  };

  qtBaseNode(qtDiagramGenerator* scene, QGraphicsItem* parent = nullptr);
  ~qtBaseNode() override;

  /// Return a dummy bounding-rect.
  ///
  /// Without this, calling this->setCursor() in the qtBaseNode constructor
  /// results in a pure virtual function call (as the subclass that implements
  /// the actual bounding-box computation has not yet been initialized).
  QRectF boundingRect() const override;

  /// Return a UUID for this node.
  virtual smtk::common::UUID nodeId() const = 0;

  /// Return the generator responsible for this node.
  qtDiagramGenerator* generator() const { return m_generator; }

  /// Return the diagram which owns the generator of this node.
  qtDiagram* diagram() const;

  /// Return the scene this node belongs to.
  qtDiagramScene* scene() const;

  /// Nodes must provide a textual name used for sorting and presentation to users.
  virtual std::string name() const = 0;

  /// Set/get how much data the node should render inside its boundary.
  virtual void setContentStyle(ContentStyle cs);
  ContentStyle contentStyle() const { return m_contentStyle; }

  /// Deal with updates (e.g., a configuration change that affects the
  /// node's visual appearance or interaction style).
  ///
  /// This method is invoked when an operation makes changes to the data
  /// the node represents.
  // was: virtual void updateToMatchModifiedTask(){};
  virtual void dataUpdated(){};

Q_SIGNALS:
  void nodeResized();
  void nodeMoved();

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  /// Update the node bounds to fit its content.
  virtual int updateSize();

  ///\brief Adds the node to the scene
  ///
  /// NOTE: since qtBaseNode class does not have any UI geometry associated
  /// with it, it will not automatically add itself to the Task Scene.  Derived
  /// classes should call this method after its UI geometry has been setup
  void addToScene();

  qtDiagramGenerator* m_generator{ nullptr };
  ContentStyle m_contentStyle{ ContentStyle::Minimal };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtBaseNode_h
