//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtComponentAttributeView - the UI components for Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtk_extension_qtComponentAttributeView_h
#define smtk_extension_qtComponentAttributeView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <QMap>
#include <QModelIndex>
#include <QStyledItemDelegate>

class qtComponentAttributeViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

/**\brief Provides the QT UI for a Resource Component Attribute View.
  *
  * A Component Attribute View is similar to an Attribute View in that it used to create and delete attributes.
  * However, unlike the Attribute View, an attribute is associated to only one Component of a Resource.  All appropriate
  * Components are listed in the first column of a table and their corresponding attributes (by type) are  listed in
  * the second column.
  *
  * The structure of the Component Attribute View has the following Top-Level view attributes in addition to
  * those that Attribute Views have:
  *
  * - Col1Header - The name to be used as the column 1 header if the attribute
  * does not exist "Entity" is displayed
  * - Col2Header - The name to be used as the column 2 header if the attribute
  * does not exist "Type" is displayed
  * - NoValueLabel - The label to be used to indicate that a resource component does not have an appropriate attribute associated with it
  *
  * \sa qtAttributeView
  */
namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtComponentAttributeView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtComponentAttributeView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtComponentAttributeView(const smtk::view::Information& info);
  ~qtComponentAttributeView() override;
  const QMap<QString, QList<smtk::attribute::DefinitionPtr>>& attDefinitionMap() const;
  void updateModelEntities();
  QTableWidgetItem* getSelectedItem();
  /**\brief method used to update the view when the selected row of the table widget is changed
  * broadcastSelected indicates if the view should notify the Selection Manager that a resource component
  has been selected - when the row changed is initiated by the Selection Manager this is set to false */
  void showCurrentRow(bool broadcastSelected);
  /**\brief method used when model entities are selected via the Selection System */
  void updateSelectedComponent(const std::string& selSource, smtk::view::SelectionPtr p);

  bool isEmpty() const override;

public Q_SLOTS:
  void updateUI() override;
  void onShowCategory() override;
  void showAdvanceLevelOverlay(bool show) override;
  /**\brief slot called when the user changes the type of attribute assigned to the resource component*/
  void cellChanged(int row, int column);
  /**\brief slot called when model information is changed */
  void updateModelAssociation() override;
  /**\brief slot called when the user selects a row of the table */
  void selectedRowChanged();

Q_SIGNALS:

protected:
  void buildUI() override;
  void createWidget() override;

  /**\brief Display the contents of a specific attribute. */
  void displayAttribute(smtk::attribute::AttributePtr att);
  /**\brief Gather all definitions and group them based on category.
  * This includes all definitions derived from the types specified in the view. */
  virtual void getAllDefinitions();
  /**\brief Return a persistent object that corresponds to a table widget item.*/
  smtk::resource::PersistentObjectPtr object(QTableWidgetItem* item);

private:
  qtComponentAttributeViewInternals* Internals;

}; // class

class qComponentAttributeViewComboBoxItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  qComponentAttributeViewComboBoxItemDelegate(const QStringList& vals, QObject* parent = nullptr);
  ~qComponentAttributeViewComboBoxItemDelegate() override;

  QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)
    const override;

protected:
  bool eventFilter(QObject* object, QEvent* event) override;
  QStringList m_values;
};

}; // namespace extension
}; // namespace smtk

#endif
