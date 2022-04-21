//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtReferenceItemEditor_h
#define smtk_extension_qtReferenceItemEditor_h

#include "smtk/extension/qt/qtItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QWidget>

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Observer.h"

#include <set>

class qtReferenceItemEditorInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
namespace extension
{

/// \brief qtReferenceItemEditor is a custom UI interface for
/// a smtk::attribute::ReferenceItem that uses a Combo Box.
///
/// This qtItem is used by smtk::extension::qtComponentItem and
/// smtk::extension::qtResourceItem to provide a simpler UI for
/// items that are not extensible and that hold a single value.
/// This qtItem also provides two options:
///
/// 1. The ability to restrict the potential choices to those
/// associated with the item's attribute (UseAssociations="true")
///
/// 2. The ability to create a new object (this is still under development)
///

class SMTKQTEXT_EXPORT qtReferenceItemEditor : public qtItem
{
  Q_OBJECT

public:
  qtReferenceItemEditor(const qtAttributeItemInfo& info);
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);

  ~qtReferenceItemEditor() override;
  void markForDeletion() override;

  virtual std::string selectionSourceName() { return m_selectionSourceName; }
  void setDefinitionForCreation(smtk::attribute::DefinitionPtr& def);
  void setOkToCreate(bool val) { m_okToCreate = val; }
  smtk::resource::PersistentObjectPtr object(int index);
public Q_SLOTS:
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
  // Refreshes the active children (if any)
  virtual void updateContents();

protected Q_SLOTS:
  virtual void removeObservers();
  virtual void resetHover();
  virtual void highlightOnHoverChanged(bool);

protected:
  void createWidget() override;

  // helper function to update available/current list after selection
  void updateListItemSelectionAfterChange(QList<QListWidgetItem*> selItems, QListWidget* list);

  // This widget needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  int handleOperationEvent(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  // This widget needs to handle changes made to resources as a result of resources being removed.
  // This method is used by the observation mechanism to address this via the resource manager
  void handleResourceEvent(
    const smtk::resource::Resource& resource,
    smtk::resource::EventType event);

private:
  qtReferenceItemEditorInternals* m_internals;
  std::string m_selectionSourceName;
  smtk::operation::Observers::Key m_operationObserverKey;
  smtk::resource::Observers::Key m_resourceObserverKey;
  smtk::attribute::WeakDefinitionPtr m_creationDef;
  bool m_okToCreate;
  // Should the source of possible values be restricted to those associated
  // to the Item's Attribute
  bool m_useAssociations;
  std::map<int, smtk::resource::WeakPersistentObjectPtr> m_mappedObjects;
  // This is used to indicate that the current value is not considered valid
  bool m_currentValueIsInvalid = false;
}; // class
}; // namespace extension
}; // namespace smtk

#endif
