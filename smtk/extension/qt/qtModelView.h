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
#include "smtk/extension/qt/qtEntityItemModel.h"

#include "smtk/common/UUID.h"
#include "smtk/model/SessionRef.h"

#include <QTreeView>
#include <QPoint>
#include <QMap>
#include <map>
#include <utility>

class QDropEvent;
class QMenu;

namespace smtk{  
  namespace extension {
  class qtFileItem;
  class qtModelEntityItem;
  class qtMeshSelectionItem;
  class qtModelOperationWidget;
  class qtOperatorDockWidget;

  class SMTKQTEXT_EXPORT qtModelView : public QTreeView
  {
    Q_OBJECT

  public:
    qtModelView(QWidget* p = NULL);
    ~qtModelView();

    smtk::extension::QEntityItemModel* getModel() const;
    smtk::model::DescriptivePhrasePtr currentItem() const;
    void syncEntityVisibility(
      const smtk::model::SessionPtr& sessPtr,
      const smtk::common::UUIDs& entids,
      const smtk::mesh::MeshSets& meshes,
      int vis);
    void syncEntityColor(
      const smtk::model::SessionPtr&,
      const smtk::common::UUIDs& entids,
      const smtk::mesh::MeshSets& meshes,
      const QColor& clr);
    void syncEntityVisibility(
      const smtk::common::UUID& sessid,
      const smtk::common::UUIDs& entids,
      const smtk::mesh::MeshSets& meshes,
      int vis);
    void syncEntityColor(
      const smtk::common::UUID& sessid,
      const smtk::common::UUIDs& entids,
      const smtk::mesh::MeshSets& meshes,
      const QColor& clr);

    void currentSelectionByMask(
      smtk::model::EntityRefs& selentityrefs, const smtk::model::BitFlags& entityFlags,
      smtk::model::DescriptivePhrases& selproperties,
      bool searchUp = false, smtk::mesh::MeshSets* selmeshes = NULL);
    virtual void updateWithOperatorResult(
      const smtk::model::SessionRef& sref, const smtk::model::OperatorResult& result);
    std::string determineAction(const QPoint& pPos) const;
    qtModelOperationWidget* operatorsWidget();
    void expandAllModels();

  public slots:
    bool removeSession(const smtk::model::SessionRef& sref);
    void selectItems(const smtk::common::UUIDs& selEntities,
		     const smtk::mesh::MeshSets& selMeshes,
		     bool blocksignal);
    void selectEntityItems(const smtk::common::UUIDs& selEntityRefs,
			   bool blocksignal);
    void selectEntities(const smtk::common::UUIDs& selEntIds)
    { this->selectEntityItems(selEntIds, false); }
    void selectMeshItems(const smtk::mesh::MeshSets& selMeshes,
			 bool blocksignal);
    void selectMeshes(const smtk::mesh::MeshSets& selMeshes)
    { this->selectMeshItems(selMeshes, false); }

    void showContextMenu(const QPoint &p);
    void showContextMenu(const QModelIndex &idx, const QPoint &p = QPoint());
    void operatorInvoked();
    void toggleEntityVisibility( const QModelIndex& );
    void changeEntityColor( const QModelIndex&);
    void changeEntityName( const QModelIndex& idx);
    void onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts);
    bool requestOperation(const smtk::model::OperatorPtr& brOp, bool launchUI);
    bool requestOperation(const std::string& opName,
			  const smtk::common::UUID& sessionId, bool launchOp);
    virtual void onOperationPanelClosing();

  signals:
    void selectionChanged(const smtk::model::EntityRefs& selEntityRefs,
                          const smtk::mesh::MeshSets& selmeshes,
                          const smtk::model::DescriptivePhrases& selproperties);

    void operationRequested(const smtk::model::OperatorPtr& brOp);
    void operationCancelled(const smtk::model::OperatorPtr& brOp);
    void operationFinished(const smtk::model::OperatorResult&);
    void fileItemCreated(smtk::extension::qtFileItem* fileItem);
    void modelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
    void visibilityChangeRequested(const QModelIndex&);
    void colorChangeRequested(const QModelIndex&);
    void meshSelectionItemCreated(smtk::extension::qtMeshSelectionItem*,
				  const std::string& opName,
				  const smtk::common::UUID& uuid);

  protected slots:
      virtual void removeEntityGroup(const smtk::model::Model& modelEnt,
				     const smtk::model::SessionRef& session,
				     const QList<smtk::model::Group>& groups);
    virtual void removeFromEntityGroup(const smtk::model::Model& modelEnt,
				       const smtk::model::SessionRef& session,
				       const smtk::model::Group& grp,
				       const smtk::model::EntityRefs& entities);
    virtual void newIndexAdded(const QModelIndex & newidx);

  protected:
    // If 'Delete' button is pressed, invoke proper operation if possible.
    // For example, in discrete session, user can delete a group,
    // or remove members from a group by selecting them then press delete key.
    virtual void keyPressEvent(QKeyEvent*);

    virtual void mouseReleaseEvent( QMouseEvent * );

    template<typename T>
      T owningEntityAs(const QModelIndex &idx) const;
    template<typename T>
      T owningEntityAs(const smtk::model::DescriptivePhrasePtr &dp) const;
    void owningEntitiesByMask (smtk::model::DescriptivePhrasePtr inDp,
			       smtk::model::EntityRefs& selentityrefs,
			       smtk::model::BitFlags entityFlags);

    bool hasSessionOp(const smtk::model::SessionRef& brSession,
		      const std::string& opname);
    bool hasSessionOp(const QModelIndex& idx,
		      const std::string& opname);
    smtk::model::OperatorPtr getOp(const QModelIndex& idx, const std::string& opname);
    smtk::model::OperatorPtr getOp(const smtk::model::SessionPtr& brSession, const std::string& opname);

    //Description:
    // Support for customized drag-n-drop events
    virtual Qt::DropActions supportedDropActions() const;
    void dragEnterEvent( QDragEnterEvent * event );
    void dragMoveEvent( QDragMoveEvent * event );
    virtual void startDrag ( Qt::DropActions supportedActions );
    virtual void dropEvent(QDropEvent* event);

    // Description:
    // Customized selection related methods
    virtual void  selectionChanged (const QItemSelection & selected,
				    const QItemSelection & deselected );
    virtual void selectionHelper(QEntityItemModel* qmodel,
				 const QModelIndex& parent,
				 const smtk::common::UUIDs& selEntities,
				 const smtk::mesh::MeshSets& selMeshes,
				 QItemSelection& selItems);
    void expandToRoot(QEntityItemModel* qmodel, const QModelIndex& idx);
    void recursiveSelect (const smtk::model::DescriptivePhrasePtr& dPhrase,
			  smtk::model::EntityRefs& selentityrefs,
			  smtk::model::BitFlags entityFlags,
        smtk::model::DescriptivePhrases& selproperties,
        bool exactMatch,
			  smtk::mesh::MeshSets* selmeshes = NULL);
    void selectMeshes (const smtk::model::DescriptivePhrasePtr& dPhrase,
		       smtk::mesh::MeshSets* selmeshes);

    smtk::model::Group groupParentOfIndex(const QModelIndex& qidx);
    smtk::model::Group groupParent(const smtk::model::DescriptivePhrasePtr& phrase);
    bool initOperator(smtk::model::OperatorPtr op);
    void initOperatorsDock(const std::string& opName,
			   smtk::model::SessionPtr session);
    qtOperatorDockWidget* operatorsDock();

/*
  void findIndexes(
    QEntityItemModel* qmodel,
    const QModelIndex& parentIdx,
    const smtk::common::UUIDs& selEntities,
    QModelIndexList& foundIndexes);
*/
    bool setEntityVisibility(const smtk::model::EntityRefs& selentityrefs,
			     const smtk::mesh::MeshSets& selmeshes,
			     int vis, smtk::model::OperatorPtr op);
    bool setEntityColor(const smtk::model::EntityRefs& selentityrefs,
			const smtk::mesh::MeshSets& selmeshes,
			const QColor& newcolor, smtk::model::OperatorPtr brOp);

    QMenu* m_ContextMenu;
    qtOperatorDockWidget* m_OperatorsDock;
    qtModelOperationWidget* m_OperatorsWidget;
    std::map<std::string, std::pair<std::vector<std::string>, std::map<std::string, std::string> > > m_sessionInfo;
  };

  } // namespace extension
} // namespace smtk

#endif // __smtk_extension_qtModelView_h
