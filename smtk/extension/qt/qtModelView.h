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

#ifndef _qtModelView_h
#define _qtModelView_h

#include "smtk/extension/qt/QtSMTKExports.h"
#include "smtk/extension/qt/qtEntityItemModel.h"

#include "smtk/common/UUID.h"
#include "smtk/model/SessionRef.h"

#include <QTreeView>
#include <QPoint>
#include <QMap>

class QDropEvent;
class QMenu;
class QDockWidget;

namespace smtk {
 namespace attribute {
  class qtFileItem;
 }
}

namespace smtk {
  namespace model {

class DescriptivePhrase;
class qtModelOperationWidget;

class QTSMTK_EXPORT qtModelView : public QTreeView
{
  Q_OBJECT

public:
  qtModelView(QWidget* p = NULL);
  ~qtModelView();

  smtk::model::QEntityItemModel* getModel() const;
  DescriptivePhrasePtr currentItem() const;
  void addGroup(BitFlags flag, const std::string& name);
  void syncEntityVisibility(
    const QMap<smtk::model::SessionPtr, smtk::common::UUIDs>& brEntities,
    int vis);
  void syncEntityColor(
    const QMap<smtk::model::SessionPtr, smtk::common::UUIDs>& brEntities,
    const QColor& clr);

public slots:
  void selectEntities(const smtk::common::UUIDs& selEntityRefs);
  virtual void removeFromGroup(const QModelIndex& qidx);
  virtual void removeSelected();
  void showContextMenu(const QPoint &p);
  void operatorInvoked();
  void toggleEntityVisibility( const QModelIndex& );
  void changeEntityColor( const QModelIndex&);

signals:
  void entitiesSelected(const smtk::model::EntityRefs& selEntityRefs);
  void operationRequested(const smtk::model::OperatorPtr& brOp);
  void operationFinished(const smtk::model::OperatorResult&);
  void fileItemCreated(smtk::attribute::qtFileItem* fileItem);
  void visibilityChangeRequested(const QModelIndex&);
  void colorChangeRequested(const QModelIndex&);

protected:

  SessionRef getSessionRef(
    const QModelIndex &idx) const;
  OperatorPtr getSetPropertyOp(const QModelIndex& idx);
  OperatorPtr getSetPropertyOp(smtk::model::SessionPtr session);
  // Description:
  // Support for customized drag-n-drop events
  virtual Qt::DropActions supportedDropActions() const;
  void dragEnterEvent( QDragEnterEvent * event );
  void dragMoveEvent( QDragMoveEvent * event );
  virtual void startDrag ( Qt::DropActions supportedActions );
  virtual void dropEvent(QDropEvent* event);

  // Description:
  // Customized selection related methods
  virtual void  selectionChanged (
    const QItemSelection & selected, const QItemSelection & deselected );
  virtual void selectionHelper(
  QEntityItemModel* qmodel,
    const QModelIndex& parent,
    const smtk::common::UUIDs& selEntities,
    QItemSelection& selItems);
  void expandToRoot(QEntityItemModel* qmodel, const QModelIndex& idx);
  void recursiveSelect (smtk::model::DescriptivePhrasePtr dPhrase,
    smtk::model::EntityRefs& selentityrefs, BitFlags entityFlags);

  smtk::model::Group groupParentOfIndex(const QModelIndex& qidx);
  bool initOperator(smtk::model::OperatorPtr op);
  QDockWidget* operatorsDock(
    const std::string& opName, smtk::model::SessionPtr session);
/*
  void findIndexes(
    QEntityItemModel* qmodel,
    const QModelIndex& parentIdx,
    const smtk::common::UUIDs& selEntities,
    QModelIndexList& foundIndexes);
*/
  bool setEntityVisibility(
    const smtk::model::EntityRefs& selentityrefs,
    int vis, OperatorPtr op);
  bool setEntityColor(
  const smtk::model::EntityRefs& selentityrefs,
  const QColor& newcolor, OperatorPtr brOp);

  QMenu* m_ContextMenu;
  QDockWidget* m_OperatorsDock;
  qtModelOperationWidget* m_OperatorsWidget;
};

  } // namespace model
} // namespace smtk

#endif // !_qtModelView_h
