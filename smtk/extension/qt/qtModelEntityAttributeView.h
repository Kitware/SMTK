//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelEntityAttributeView - the UI components for Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtModelEntityAttributeView_h
#define __smtk_extension_qtModelEntityAttributeView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <QMap>
#include <QModelIndex>
#include <QStyledItemDelegate>

class qtModelEntityAttributeViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

/**\brief Provides the QT UI for a Model Entity Attribute View.
  *
  * A Model Entity Attribute View is similar to an Attribute View in that it used to create and delete attributes.
  * However, unlike the Attribute View, an attribute is associated to only one model entity.  All appropriate
  * model entities are listed in the first column of a table and their corresponding attributes (by type) are  listed in
  * the second column.
  *
  * The structure of the Model Entity Attribute View has the following Top-Level view attributes in additon to
  * those that Attribute Views have:
  *
  * - Type = "ModelEntity"
  * - Col1Header - The name to be used as the column 1 header if the attribute
  * does not exist "Model Entity" is displayed
  * - Col2Header - The name to be used as the column 2 header if the attribute
  * does not exist "Type" is displayed
  * - NoValueLabel - The label to be used to indicate that a model entity does not have an appropriate attribute associated with it
  *
  * \sa qtAttributeView
  */
namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtModelEntityAttributeView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtModelEntityAttributeView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtModelEntityAttributeView(const smtk::view::Information& info);
  virtual ~qtModelEntityAttributeView();
  const QMap<QString, QList<smtk::attribute::DefinitionPtr> >& attDefinitionMap() const;
  void updateModelEntities();
  QTableWidgetItem* getSelectedItem();
  /**\brief method used to update the view when the selected row of the table widget is changed
  * broadcastSelected indicates if the view should notify the Selection Manager that a model entity
  has been selected - when the row changed is initiated by the Selection Manager this is set to false */
  void showCurrentRow(bool broadcastSelected);
  /**\brief method used when model entities are selected via the Selection System */
  void updateSelectedModelEntity(const std::string& selSource, smtk::view::SelectionPtr p);

  bool isEmpty() const override;

public slots:
  void updateUI() override;
  void onShowCategory() override;
  void showAdvanceLevelOverlay(bool show) override;
  /**\brief slot called when the user changes the type of attribute assigned to the model entity*/
  void cellChanged(int row, int column);
  /**\brief slot called when model information is changed */
  void updateModelAssociation() override;
  /**\brief slot called when the user selects a row of the table */
  void selectedRowChanged();

signals:

protected:
  void buildUI() override;
  void createWidget() override;

  /**\brief Display the contents of a specific attribute. */
  void displayAttribute(smtk::attribute::AttributePtr att);
  /**\brief Gather all definitions and group them based on category.
  * This includes all definitions derived from the types specified in the view. */
  virtual void getAllDefinitions();
  /**\brief Return a set of all appropriate objects based on the view's model entity mask.*/
  std::set<smtk::resource::PersistentObjectPtr> associatableObjects() const;
  /**\brief Return a presistent object that cooresponds to a table widget item.*/
  smtk::resource::PersistentObjectPtr object(QTableWidgetItem* item);

protected slots:
  void selectionMade();

private:
  qtModelEntityAttributeViewInternals* Internals;

}; // class

class qModelEntityAttributeViewComboBoxItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  qModelEntityAttributeViewComboBoxItemDelegate(const QStringList& vals, QObject* parent = 0);
  ~qModelEntityAttributeViewComboBoxItemDelegate();

  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

signals:
  void choiceMade();

protected:
  QStringList m_values;
};

}; // namespace attribute
}; // namespace smtk

#endif
