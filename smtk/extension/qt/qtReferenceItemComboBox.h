//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtReferenceItemComboBox_h
#define __smtk_extension_qtReferenceItemComboBox_h

#include "smtk/extension/qt/qtItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QWidget>

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Observer.h"

#include <set>

class qtReferenceItemComboBoxInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
namespace extension
{

/// \brief qtReferenceItemComboBox is a custom UI interface for
/// a smtk::attribute::ReferenceItem that uses a Combo Box.
///
/// This qtItem is used by smtk::extension::qtComponentItem and
/// smtk::extension::qtResourceItem to provide a simplier UI for
/// items that are not extensible and that hold a single value.
/// This qtItem also provides two options:
///
/// 1. The ability to restrict the potential choices to those
/// associated with the item's attribute (UseAssociations="true")
///
/// 2. The ability to create a new object (this is still under development)
///

class SMTKQTEXT_EXPORT qtReferenceItemComboBox : public qtItem
{
  Q_OBJECT

public:
  qtReferenceItemComboBox(const qtAttributeItemInfo& info);
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);

  virtual ~qtReferenceItemComboBox();
  void markForDeletion() override;

  virtual std::string selectionSourceName() { return m_selectionSourceName; }
  void setDefinitionForCreation(smtk::attribute::DefinitionPtr& def);
  void setOkToCreate(bool val) { m_okToCreate = val; }
  smtk::resource::PersistentObjectPtr object(int index);
  std::set<smtk::resource::PersistentObjectPtr> checkUniquenessCondition(
    const std::set<smtk::resource::PersistentObjectPtr>& objSet) const;
public slots:
  void updateItemData() override;
  void highlightItem(int index);
  void selectItem(int index);
  void setOutputOptional(int state);
  void itemChanged(smtk::attribute::ItemPtr item);
  // Refresh the association information for the current attribute.  If ignoreResource is specified
  // the corresponding resource will not participate in determining which object can be associated.
  // The main use case would be updating the widget because a resource is about to be removed from the
  // system.  Since it is still in memory we needed a way to ignore it
  virtual void updateChoices(const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());

protected slots:
  virtual void removeObservers();
  virtual void resetHover();
  virtual void highlightOnHoverChanged(bool);

protected:
  void createWidget() override;
  // Get a set of objects that could be associated with the current attribute.  If ignoreResource is specified
  // the corresponding resource will not participate in determining which object can be associated.
  // The main use case would be updating the widget because a resource is about to be removed from the
  // system.  Since it is still in memory we needed a way to ignore it
  std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
    const smtk::common::UUID& ignoreResource = smtk::common::UUID::null()) const;

  // helper function to update available/current list after selection
  void updateListItemSelectionAfterChange(QList<QListWidgetItem*> selItems, QListWidget* list);

  // This widget needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  int handleOperationEvent(const smtk::operation::Operation& op, smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  // This widget needs to handle changes made to resources as a result of resources being removed.
  // This method is used by the observation mechanism to address this via the resource manager
  void handleResourceEvent(
    const smtk::resource::Resource& resource, smtk::resource::EventType event);

private:
  qtReferenceItemComboBoxInternals* Internals;
  std::string m_selectionSourceName;
  smtk::operation::Observers::Key m_operationObserverKey;
  smtk::resource::Observers::Key m_resourceObserverKey;
  smtk::attribute::WeakDefinitionPtr m_creationDef;
  bool m_okToCreate;
  bool m_useAssociations;
  std::map<int, smtk::resource::WeakPersistentObjectPtr> m_mappedObjects;
}; // class
}; // namespace attribute
}; // namespace smtk

#endif
