//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAssociationWidget - the Attribute-Model association widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtAssociationWidget_h
#define __smtk_extension_qtAssociationWidget_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QWidget>

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Observer.h"

#include <set>

class qtAssociationWidgetInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
namespace extension
{
class qtBaseView;
class SMTKQTEXT_EXPORT qtAssociationWidget : public QWidget
{
  Q_OBJECT

public:
  qtAssociationWidget(QWidget* p, qtBaseView* view);
  virtual ~qtAssociationWidget();
  bool hasSelectedItem();
  // when register the instance with qtSelectionManager, also add the memory address.
  virtual std::string selectionSourceName() { return m_selectionSourceName; }

public slots:
  // Display the association information to a specific attribute
  virtual void showEntityAssociation(smtk::attribute::AttributePtr theAtt);
  // Refresh the association information for the current attribute;
  virtual void refreshAssociations();

signals:
  void attAssociationChanged();

protected slots:
  virtual void onRemoveAssigned();
  virtual void onAddAvailable();
  virtual void removeObservers();

protected:
  virtual void initWidget();
  QList<QListWidgetItem*> getSelectedItems(QListWidget* theList) const;
  virtual void removeItem(QListWidget*, QListWidgetItem*);
  smtk::attribute::AttributePtr getAttribute(QListWidgetItem* item);
  smtk::attribute::AttributePtr getSelectedAttribute(QListWidgetItem*);

  smtk::resource::PersistentObjectPtr object(QListWidgetItem* item);
  smtk::resource::PersistentObjectPtr selectedObject(QListWidgetItem*);
  std::set<smtk::resource::PersistentObjectPtr> associatableObjects() const;
  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  //for now we append model name to currentList
  virtual QListWidgetItem* addObjectAssociationListItem(QListWidget* theList,
    const smtk::resource::PersistentObjectPtr& object, bool sort = true,
    bool appendModelName = false);

  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  virtual QListWidgetItem* addAttributeAssociationItem(
    QListWidget* theList, smtk::attribute::AttributePtr att, bool sort = true);

  // helper function to update available/current list after selection
  void updateListItemSelectionAfterChange(QList<QListWidgetItem*> selItems, QListWidget* list);

  // This widget needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  int handleOperationEvent(smtk::operation::OperationPtr op, smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  // This widget needs to handle changes made to resources as a result of resources being removed.
  // This method is used by the observation mechanism to address this via the resource manager
  void handleResourceEvent(smtk::resource::Resource::Ptr resource, smtk::resource::EventType event);

private:
  qtAssociationWidgetInternals* Internals;
  std::string m_selectionSourceName;
  smtk::operation::Observers::Key m_operationObserverKey;
  smtk::resource::Observers::Key m_resourceObserverKey;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
