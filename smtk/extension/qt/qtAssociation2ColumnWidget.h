//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAssociation2ColumnWidget_h
#define smtk_extension_qtAssociation2ColumnWidget_h

#include "smtk/extension/qt/qtAssociationWidget.h"
#include <QString>

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Observer.h"

#include <set>

class qtAssociation2ColumnWidgetInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
namespace extension
{
class qtBaseView;
class SMTKQTEXT_EXPORT qtAssociation2ColumnWidget : public qtAssociationWidget
{
  Q_OBJECT

public:
  qtAssociation2ColumnWidget(QWidget* p, qtBaseView* view);
  ~qtAssociation2ColumnWidget() override;
  bool hasSelectedItem() override;
  // when register the instance with qtSelectionManager, also add the memory address.
  virtual std::string selectionSourceName() { return m_selectionSourceName; }
  void leaveEvent(QEvent*) override;
  bool isValid() const override;
  void setAllAssociatedMode(bool val) { m_allAssociatedMode = val; }
  bool allAssociatedMode() const { return m_allAssociatedMode; }
  void setAllAssociationWarning(const std::string& message)
  {
    m_allAssociatedWarning = message.c_str();
  }
  void setCurrentLabel(const std::string& message);
  void setAvailableLabel(const std::string& message);
  void setTitleLabel(const std::string& message);

public Q_SLOTS:
  // Display the association information to a specific attribute
  void showEntityAssociation(smtk::attribute::AttributePtr theAtt) override;
  // Display the association information to a specific definition
  void showEntityAssociation(smtk::attribute::DefinitionPtr theDef) override;
  // Refresh the association information for the current attribute.  If ignoreResource is specified
  // the corresponding resource will not participate in determining which object can be associated.
  // The main use case would be updating the widget because a resource is about to be removed from the
  // system.  Since it is still in memory we needed a way to ignore it
  virtual void refreshAssociations(
    const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());

protected Q_SLOTS:
  virtual void onRemoveAssigned();
  virtual void onAddAvailable();
  virtual void removeObservers();
  virtual void hoverRow(const QModelIndex& idx);
  virtual void resetHover();
  virtual void highlightOnHoverChanged(bool);
  virtual void onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*);

protected:
  virtual void initWidget();
  QList<QListWidgetItem*> getSelectedItems(QListWidget* theList) const;
  virtual void removeItem(QListWidget*, QListWidgetItem*);
  smtk::attribute::AttributePtr getAttribute(QListWidgetItem* item);
  smtk::attribute::AttributePtr getSelectedAttribute(QListWidgetItem*);

  smtk::resource::PersistentObjectPtr object(QListWidgetItem* item);
  smtk::resource::PersistentObjectPtr selectedObject(QListWidgetItem*);
  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  //for now we append model name to currentList
  virtual QListWidgetItem* addObjectAssociationListItem(
    QListWidget* theList,
    const smtk::resource::PersistentObjectPtr& object,
    bool sort = true,
    bool appendModelName = false);

  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  virtual QListWidgetItem* addAttributeAssociationItem(
    QListWidget* theList,
    smtk::attribute::AttributePtr att,
    bool sort = true);

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
  int handleResourceEvent(
    const smtk::resource::Resource& resource,
    smtk::resource::EventType event);
  // Used to determine if an alert icon should be displayed and why
  void updateAssociationStatus(const smtk::attribute::Attribute* att);

  // Sets the validity of the widget
  void setIsValid(bool val);

private:
  qtAssociation2ColumnWidgetInternals* m_internals;
  std::string m_selectionSourceName;
  smtk::operation::Observers::Key m_operationObserverKey;
  smtk::resource::Observers::Key m_resourceObserverKey;
  bool m_allAssociatedMode;
  bool m_isValid;
  QString m_allAssociatedWarning;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
