//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAttributeView_h
#define smtk_extension_qtAttributeView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/view/Configuration.h"

#include <QItemSelection>
#include <QMap>
#include <QModelIndex>

class qtAttributeViewInternals;

class QAbstractItemDelegate;
class QAction;
class QKeyEvent;
class QStandardItem;
class QTableWidgetItem;
class QTableWidget;
class QToolBar;

namespace smtk
{
namespace extension
{
class qtAssociationWidget;
class qtBaseView;

///\brief Qt implementation for an Attribute View
///
/// Attribute Views allow users to be able to create/modify/delete attributes based
/// on a set of Definition types.  Note that if a provided Definition is the base type
/// for others, then all derived attributes can be displayed/created as well
class SMTKQTEXT_EXPORT qtAttributeView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtAttributeView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtAttributeView(const smtk::view::Information& info);
  ~qtAttributeView() override;
  const QMap<QString, QList<smtk::attribute::DefinitionPtr>>& attDefinitionMap() const;

  QStandardItem* getSelectedItem();
  int currentViewBy();
  virtual void createNewAttribute(smtk::attribute::DefinitionPtr attDef);
  bool isEmpty() const override;
  bool isValid() const override;

  void setSearchBoxText(const std::string& text) { m_searchBoxText = text; }
  const std::string& searchBoxText() const { return m_searchBoxText; }

  void setSearchBoxVisibility(bool mode) { m_allAssociatedMode = mode; }
  bool searchBoxVisibility() const { return m_searchBoxVisibility; }

  void setHideAssociations(bool mode) { m_hideAssociations = mode; }
  bool hideAssociations() const { return m_hideAssociations; }

  void setRequireAllAssociated(bool mode) { m_allAssociatedMode = mode; }
  bool requireAllAssociated() const { return m_allAssociatedMode; }

  void setAttributeNamesConstant(bool mode) { m_disableNameField = mode; }
  bool attributeNamesConstant() const { return m_disableNameField; }

  void setAttributeNameRegex(const std::string& pattern) { m_attributeNameRegex = pattern; }
  const std::string& attributeNameRegex() const { return m_attributeNameRegex; }

  smtk::attribute::DefinitionPtr getCurrentDef() const;

  // Returns true if the Definition matches any of the View's Definitions
  bool matchesDefinitions(const smtk::attribute::DefinitionPtr& def) const;
  enum enumViewBy
  {
    VIEWBY_Attribute = 0,
    VIEWBY_PROPERTY
  };
public Q_SLOTS:
  void onViewBy();
  void onViewByWithDefinition(smtk::attribute::DefinitionPtr attDef);
  void updateUI() override;
  void onShowCategory() override;
  void onListBoxSelectionChanged();
  void onAttributeValueChanged(QTableWidgetItem*);
  void onAttributeNameChanged(QStandardItem*);
  void onCreateNew();
  void onCopySelected();
  void onDeleteSelected();
  void updateAssociationEnableState(smtk::attribute::AttributePtr);
  void updateModelAssociation() override;
  void onListBoxClicked(const QModelIndex& item);
  void onAttributeItemChanged(QStandardItem* item);
  void childrenResized() override;
  void showAdvanceLevelOverlay(bool show) override;
  void associationsChanged();
  void onItemChanged(qtItem* item);
  void onDefinitionChanged(int);

Q_SIGNALS:
  void numOfAttributesChanged();
  void attColorChanged();
  void attAssociationChanged();
  // signal to indicate that a different attribute has been selected
  void attributeSelected(smtk::attribute::AttributePtr att);
  void definitionSelected(smtk::attribute::DefinitionPtr def);

protected:
  void createWidget() override;
  virtual smtk::extension::qtAssociationWidget* createAssociationWidget(
    QWidget* parent,
    qtBaseView* view);
  // Methods for fetching attributes corresponding to either a StandardItem or ModelIndex.
  smtk::attribute::AttributePtr getAttributeFromItem(const QStandardItem* item);
  smtk::attribute::AttributePtr getAttributeFromIndex(const QModelIndex& index);
  smtk::attribute::Attribute* getRawAttributeFromItem(const QStandardItem* item);
  smtk::attribute::Attribute* getRawAttributeFromIndex(const QModelIndex& index);
  QStandardItem* getItemFromAttribute(smtk::attribute::Attribute* attribute);

  ///\brief Method used to delete an attribute from its resource
  virtual bool deleteAttribute(smtk::attribute::AttributePtr att);

  smtk::attribute::AttributePtr getSelectedAttribute();
  QStandardItem* addAttributeListItem(smtk::attribute::AttributePtr childData);
  void updateTableWithAttribute(smtk::attribute::AttributePtr dataItem);
  void addComparativeProperty(QStandardItem* current, smtk::attribute::DefinitionPtr attDef);

  void updateChildWidgetsEnableState(smtk::attribute::ItemPtr linkedData, QTableWidgetItem* item);
  void
  updateItemWidgetsEnableState(smtk::attribute::ItemPtr linkedData, int& startRow, bool enabled);
  virtual void getAllDefinitions();

  void initSelectAttCombo(smtk::attribute::DefinitionPtr attDef);
  void
  insertTableColumn(QTableWidget* wTable, int insertCol, const QString& title, int advancedlevel);
  // Determines if an alert icon should be displayed next to the attribute in the list
  void updateAttributeStatus(smtk::attribute::Attribute* att);
  // This View needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  virtual int handleOperationEvent(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  QToolBar* toolBar();

  //Needed for remove or insert
  QAction* addAction();
  QAction* copyAction();
  QAction* deleteAction();

  void setTableItemDelegate(QAbstractItemDelegate* delegate);
  void setTableColumnItemDelegate(int column, QAbstractItemDelegate* delegate);
  void setTableRowItemDelegate(int row, QAbstractItemDelegate* delegate);
  void triggerEdit(const QModelIndex& index);
  int numOfAttributes();
  const smtk::view::Configuration::Component& findStyle(
    const smtk::attribute::DefinitionPtr& def,
    bool isOriginalDef = true);

private:
  qtAttributeViewInternals* m_internals;
  bool m_hideAssociations;
  bool m_allAssociatedMode; //!< Indicates that all potential objects that can be associated must be
  bool m_associationWidgetIsUsed; // <! Indicates if the association widget is currently being used
  bool m_disableNameField;        //!< Indicates that attribute names can not be modified
  bool m_searchBoxVisibility;     //!< Indicates if the search box should be displayed
  std::string m_searchBoxText; //!< Text to be displayed in the search box when no text is entered
  std::string m_attributeNameRegex; //!< Regex pattern for attribute names
};
}; // namespace extension
}; // namespace smtk

#endif
