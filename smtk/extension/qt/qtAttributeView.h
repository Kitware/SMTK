//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeView - the UI components for Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtAttributeView_h
#define __smtk_extension_qtAttributeView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtSelectionManager.h"

#include <QMap>
#include <QModelIndex>

class qtAttributeViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtAttributeView : public qtBaseView
{
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const ViewInfo& info);
  qtAttributeView(const ViewInfo& info);
  virtual ~qtAttributeView();
  const QMap<QString, QList<smtk::attribute::DefinitionPtr> >& attDefinitionMap() const;

  QTableWidgetItem* getSelectedItem();
  int currentViewBy();
  virtual void createNewAttribute(smtk::attribute::DefinitionPtr attDef);
  // Return either the def's label if set else it returns it's type
  static std::string getDefText(smtk::attribute::DefinitionPtr attDef);

  enum enumViewBy
  {
    VIEWBY_Attribute = 0,
    VIEWBY_PROPERTY
  };
public slots:
  void onViewBy(int);
  void onViewByWithDefinition(int viewBy, smtk::attribute::DefinitionPtr attDef);
  void onShowCategory() override;
  void onListBoxSelectionChanged();
  void onAttributeValueChanged(QTableWidgetItem*);
  void onAttributeNameChanged(QTableWidgetItem*);
  void onCreateNew();
  void onCopySelected();
  void onDeleteSelected();
  void updateAssociationEnableState(smtk::attribute::AttributePtr);
  void updateModelAssociation() override;
  void onListBoxClicked(QTableWidgetItem* item);
  void onAttributeCellChanged(int, int);
  void onPropertyDefSelected();
  void attributeFilterChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
  void propertyFilterChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
  void childrenResized() override;
  void showAdvanceLevelOverlay(bool show) override;

signals:
  void numOfAttributesChanged();
  void attColorChanged();
  void attAssociationChanged();
  void relaySelectionToAssiocationWidget(const smtk::model::EntityRefs& selEntities,
    const smtk::mesh::MeshSets& selMeshes, const smtk::model::DescriptivePhrases& selDPs,
    const std::string& selectionSource);

protected:
  void createWidget() override;
  smtk::attribute::AttributePtr getAttributeFromItem(QTableWidgetItem* item);
  smtk::attribute::ItemPtr getAttributeItemFromItem(QTableWidgetItem* item);

  smtk::attribute::AttributePtr getSelectedAttribute();
  QTableWidgetItem* addAttributeListItem(smtk::attribute::AttributePtr childData);
  void updateTableWithAttribute(smtk::attribute::AttributePtr dataItem);
  void addComparativeProperty(QStandardItem* current, smtk::attribute::DefinitionPtr attDef);

  void updateChildWidgetsEnableState(smtk::attribute::ItemPtr linkedData, QTableWidgetItem* item);
  void updateItemWidgetsEnableState(
    smtk::attribute::ItemPtr linkedData, int& startRow, bool enabled);
  virtual void getAllDefinitions();

  virtual void updateTableWithProperties();
  virtual void removeComparativeProperty(const QString& propertyName);
  void initSelectionFilters();
  void initSelectAttCombo(smtk::attribute::DefinitionPtr attDef);
  void initSelectPropCombo(smtk::attribute::DefinitionPtr attDef);
  void addComparativeAttribute(smtk::attribute::AttributePtr att);
  void removeComparativeAttribute(smtk::attribute::AttributePtr att);
  void insertTableColumn(
    QTableWidget* wTable, int insertCol, const QString& title, int advancedlevel);

  // update the selection based on qtSelectionManager
  void updateSelectionOfEntities();

private:
  qtAttributeViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
