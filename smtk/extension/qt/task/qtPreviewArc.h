//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtPreviewArc_h
#define smtk_extension_qtPreviewArc_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>

namespace smtk
{
namespace extension
{

class qtTaskEditor;
class qtTaskScene;
class qtBaseTaskNode;

/**\brief A graphics item that represents an arc as it is being constructed.
  *
  */
class SMTKQTEXT_EXPORT qtPreviewArc
  : public QObject
  , public QGraphicsPathItem
{
  Q_OBJECT

public:
  using Superclass = QGraphicsPathItem;

  /// Arcs between nodes indicate a required ordering of tasks.
  enum class ArcStatus : int
  {
    Invalid, //!< An arc between the selected tasks is disallowed.
    Valid    //!< An arc between the selected tasks is allowed.
  };

  qtPreviewArc(
    qtTaskScene* scene,
    const std::shared_ptr<smtk::operation::Manager>& operationManager,
    smtk::string::Token arcType = smtk::string::Token(),
    const std::string& arcOperation = std::string(),
    QGraphicsItem* parent = nullptr);
  ~qtPreviewArc() override;

  qtTaskScene* scene() const { return m_scene; }
  smtk::string::Token arcType() const { return m_arcType; }
  std::string arcOperation() const { return m_arcOperation; }

  static ArcStatus arcStatusEnum(const std::string& enumerant, bool* match = nullptr);
  static std::string arcStatusName(ArcStatus enumerant);

  /// Set/get the predecessor node for the arc to be created.
  void setPredecessor(qtBaseTaskNode* predecessor);
  qtBaseTaskNode* predecessor() const { return m_predecessor; }

  /// Set/get the successor node for the arc to be created.
  /// You **must** call setPredecessor with a non-null value before calling this method.
  void setSuccessor(qtBaseTaskNode* successor, bool updatePoints = true);
  qtBaseTaskNode* successor() const { return m_successor; }

  /// Confirm the predecessor node.
  ///
  /// During arc construction, the predecessor node will be set repeatedly
  /// as the pointer moves over each task. When the user clicks, this method
  /// is called to indicate that the pointer should now control the successor
  /// node.
  void confirmPredecessorNode(bool confirm = true);

  /// Return whether the predecessor node has been confirmed (true) or not (false).
  bool isPredecessorConfirmed() const { return m_predecessorConfirmed; }

  /// Confirm the successor node.
  ///
  /// During arc construction, once the user clicks to confirm the current
  /// successor node (underneath the pointer), this method is invoked.
  /// It launches the operation associated with the arc and resets this
  /// class in preparation to create the next arc.
  ///
  /// Note that unlike \a confirmPredecessorNode(), this method does not
  /// store any status variable since once the operation is launched, the
  /// predecessor and successor are both reset. You cannot "unconfirm" a
  /// successor node – once confirmed the operation is immediately launched.
  void confirmSuccessorNode();

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  int updateArcPoints();

  /// Set the arc and operation type.
  /// Users may change this any time before the arc's successor node is confirmed.
  ///
  /// This returns true when a change was made and false otherwise.
  /// Note that if \a operationType is non-empty and the operation cannot be
  /// created, this method will reject the change and return false.
  bool setArcType(smtk::string::Token arcType, const std::string& operationType);

protected:
  virtual void createOperation();
  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;
  /// Draw the arc into the scene.
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  qtTaskScene* m_scene{ nullptr };
  qtBaseTaskNode* m_predecessor{ nullptr };
  qtBaseTaskNode* m_successor{ nullptr };
  smtk::string::Token m_arcType;
  std::string m_arcOperation;
  QPainterPath m_predecessorMark;
  QPainterPath m_successorMark;
  QPainterPath m_computedPath;
  QPainterPath m_arrowPath;
  bool m_predecessorConfirmed{ false };
  bool m_canAssociatePredecessor{ false };
  bool m_canAssociateSuccessor{ false };
  bool m_ableToOperate{ false };
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  std::shared_ptr<smtk::operation::Operation> m_operation;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtPreviewArc_h
