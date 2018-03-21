//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelView - a tree view of smtk model.
// .SECTION Description
// .SECTION Caveats

#ifndef __smtk_extension_qtModelView_h
#define __smtk_extension_qtModelView_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/common/UUID.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/model/SessionRef.h"
#include "smtk/operation/Operation.h"

#include <QMap>
#include <QPoint>
#include <QTreeView>
#include <map>
#include <utility>

class QDropEvent;
class QMenu;

namespace smtk
{
namespace view
{
enum class SelectionAction;
}
namespace extension
{
class qtFileItem;
class qtModelOperationWidget;
class qtOperationDockWidget;

class SMTKQTEXT_EXPORT qtModelView : public QTreeView
{
  Q_OBJECT

public:
  qtModelView(QWidget* p = NULL);
  ~qtModelView();
  virtual std::string selectionSourceName() { return m_selectionSourceName; }

  qtOperationDockWidget* operatorsDock();

  std::string determineAction(const QPoint& pPos) const;
  qtModelOperationWidget* operatorsWidget();
  bool setEntityVisibility(const smtk::model::EntityRefs& selentityrefs,
    const smtk::mesh::MeshSets& selmeshes, int vis, smtk::operation::OperationPtr op);

public slots:
  void updateActiveModelByModelIndex();
  bool removeSession(const smtk::model::SessionRef& sref);

  void showContextMenu(const QPoint& p);
  void showContextMenu(const QModelIndex& idx, const QPoint& p = QPoint());
  void operatorInvoked();
  void toggleEntityVisibility(const QModelIndex&);
  void onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts);
  bool requestOperation(const smtk::operation::OperationPtr& brOp, bool launchUI);
  bool requestOperation(
    const std::string& opName, const smtk::common::UUID& sessionId, bool launchOp);
  virtual void onOperationPanelClosing();
  virtual bool showPreviousOpOrHide(bool alwaysHide = true);

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  void operationCancelled(const smtk::operation::OperationPtr& brOp);
  void operationFinished(const smtk::operation::Operation::Result&);
  void fileItemCreated(smtk::extension::qtFileItem* fileItem);
  void visibilityChangeRequested(const QModelIndex&);
  void colorChangeRequested(const QModelIndex&);

protected slots:
  // virtual void removeEntityGroup(const smtk::model::Model& modelEnt,
  // const smtk::model::SessionRef& session, const QList<smtk::model::Group>& groups);
  // virtual void removeFromEntityGroup(const smtk::model::Model& modelEnt,
  //   const smtk::model::SessionRef& session, const smtk::model::Group& grp,
  //   const smtk::model::EntityRefs& entities);
  virtual void newIndexAdded(const QModelIndex& newidx);

protected:
  // If 'Delete' button is pressed, invoke proper operation if possible.
  // For example, in discrete session, user can delete a group,
  // or remove members from a group by selecting them then press delete key.
  void keyPressEvent(QKeyEvent*) override;

  void mouseReleaseEvent(QMouseEvent*) override;

  // bool hasSessionOp(const smtk::model::SessionRef& brSession, const std::string& opname);
  // bool hasSessionOp(const QModelIndex& idx, const std::string& opname);
  // smtk::operation::OperationPtr getOp(const QModelIndex& idx, const std::string& opname);
  // smtk::operation::OperationPtr getOp(
  //   const smtk::model::SessionPtr& brSession, const std::string& opname);

  //Description:
  // Support for customized drag-n-drop events
  virtual Qt::DropActions supportedDropActions() const;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void startDrag(Qt::DropActions supportedActions) override;
  // void dropEvent(QDropEvent* event) override;

  // Description:
  // Customized selection related methods
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

  bool initOperation(smtk::operation::OperationPtr op);
  void initOperationsDock(const std::string& opName, smtk::model::SessionPtr session);

  /*
  void findIndexes(
    QEntityItemModel* qmodel,
    const QModelIndex& parentIdx,
    const smtk::common::UUIDs& selEntities,
    QModelIndexList& foundIndexes);
*/

  QMenu* m_ContextMenu;
  qtOperationDockWidget* m_OperationsDock;
  qtModelOperationWidget* m_OperationsWidget;
  std::map<std::string, std::pair<std::vector<std::string>, std::map<std::string, std::string> > >
    m_sessionInfo;
  QModelIndex m_contextMenuIndex;
  std::string m_selectionSourceName;
};

} // namespace extension
} // namespace smtk

#endif // __smtk_extension_qtModelView_h
