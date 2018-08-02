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
  virtual void showEntityAssociation(smtk::attribute::AttributePtr theAtt);
  virtual void showDomainsAssociation(std::vector<smtk::model::Group>& theDomains,
    std::vector<smtk::attribute::DefinitionPtr>& attDefs);

signals:
  void attAssociationChanged();

protected slots:
  virtual void onRemoveAssigned();
  virtual void onAddAvailable();
  virtual void onExchange();
  virtual void onNodalOptionChanged(int);
  virtual void onDomainAssociationChanged();
  virtual void onEntitySelected();

protected:
  virtual void initWidget();
  QList<QListWidgetItem*> getSelectedItems(QListWidget* theLis) const;
  virtual void removeItem(QListWidget*, QListWidgetItem*);
  smtk::attribute::AttributePtr getAttribute(QListWidgetItem* item);
  smtk::attribute::AttributePtr getSelectedAttribute(QListWidgetItem*);

  smtk::model::EntityPtr getModelEntityItem(QListWidgetItem* item);
  smtk::model::EntityPtr getSelectedModelEntityItem(QListWidgetItem*);
  smtk::model::ResourcePtr getModelResource();

  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  //for now we append model name to currentList
  virtual QListWidgetItem* addModelAssociationListItem(QListWidget* theList,
    smtk::model::EntityPtr modelItem, bool sort = true, bool appendModelName = false);

  //returns the Item it has added to the widget
  //ownership of the item is handled by the widget so no need to delete
  virtual QListWidgetItem* addAttributeAssociationItem(
    QListWidget* theList, smtk::attribute::AttributePtr att, bool sort = true);

  virtual void addDomainListItem(
    const smtk::model::Group& domainItem, QList<smtk::attribute::AttributePtr>& allAtts);

  // helper function to update available/current list after selection
  void updateListItemSelectionAfterChange(QList<QListWidgetItem*> selItems, QListWidget* list);

private:
  qtAssociationWidgetInternals* Internals;
  std::string m_selectionSourceName;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
