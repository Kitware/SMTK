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
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

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
class qtAssociationWidget;
class qtBaseView;

class SMTKQTEXT_EXPORT qtAttributeView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtAttributeView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtAttributeView(const smtk::view::Information& info);
  virtual ~qtAttributeView();
  const QMap<QString, QList<smtk::attribute::DefinitionPtr> >& attDefinitionMap() const;

  QTableWidgetItem* getSelectedItem();
  int currentViewBy();
  virtual void createNewAttribute(smtk::attribute::DefinitionPtr attDef);
  bool isEmpty() const override;
  bool isValid() const override;

  smtk::attribute::DefinitionPtr getCurrentDef() const;

  enum enumViewBy
  {
    VIEWBY_Attribute = 0,
    VIEWBY_PROPERTY
  };
public slots:
  void onViewBy();
  void onViewByWithDefinition(smtk::attribute::DefinitionPtr attDef);
  void updateUI() override;
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
  void childrenResized() override;
  void showAdvanceLevelOverlay(bool show) override;
  void associationsChanged();
  void onItemChanged(qtItem* item);

signals:
  void numOfAttributesChanged();
  void attColorChanged();
  void attAssociationChanged();
  // signal to indicate that a different attribute has been selected
  void attributeSelected(smtk::attribute::AttributePtr att);

protected:
  void createWidget() override;
  virtual smtk::extension::qtAssociationWidget* createAssociationWidget(
    QWidget* parent, qtBaseView* view);
  smtk::attribute::AttributePtr getAttributeFromItem(QTableWidgetItem* item);
  smtk::attribute::Attribute* getRawAttributeFromItem(QTableWidgetItem* item);

  smtk::attribute::AttributePtr getSelectedAttribute();
  QTableWidgetItem* addAttributeListItem(smtk::attribute::AttributePtr childData);
  void updateTableWithAttribute(smtk::attribute::AttributePtr dataItem);
  void addComparativeProperty(QStandardItem* current, smtk::attribute::DefinitionPtr attDef);

  void updateChildWidgetsEnableState(smtk::attribute::ItemPtr linkedData, QTableWidgetItem* item);
  void updateItemWidgetsEnableState(
    smtk::attribute::ItemPtr linkedData, int& startRow, bool enabled);
  virtual void getAllDefinitions();

  void initSelectAttCombo(smtk::attribute::DefinitionPtr attDef);
  void insertTableColumn(
    QTableWidget* wTable, int insertCol, const QString& title, int advancedlevel);
  // Determines if an alert icon should be displayed next to the attribute in the list
  void updateAttributeStatus(smtk::attribute::Attribute* att);
  // This View needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  virtual int handleOperationEvent(const smtk::operation::Operation& op,
    smtk::operation::EventType event, smtk::operation::Operation::Result result);

private:
  qtAttributeViewInternals* m_internals;
  bool m_hideAssociations;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
