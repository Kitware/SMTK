//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtInputsItem - UI components for attribute ValueItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef smtk_extension_qtInputsItem_h
#define smtk_extension_qtInputsItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QDoubleValidator>

class qtInputsItemInternals;
class QBoxLayout;
class QFrame;
class QLayout;
class QLineEdit;
class QPoint;

namespace smtk
{
namespace extension
{
class qtUIManager;

class SMTKQTEXT_EXPORT qtInputsItem : public qtItem
{
  Q_OBJECT

  friend class qtDiscreteValueEditor;

public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtInputsItem(const qtAttributeItemInfo& info);
  ~qtInputsItem() override;
  void setLabelVisible(bool) override;
  void unsetValue(int elementIndex);
  bool setDiscreteValue(int elementIndex, int discreteValIndex);
  /// \brief Forces the object to act as if the underlying item was modified.
  /// Used mainly by helper classes like qtDiscreteValueEditor
  void forceUpdate();
  bool isFixedWidth() const override;
  bool eventFilter(QObject* filterObj, QEvent* ev) override;

public Q_SLOTS:
  void setOutputOptional(int);
  void onExpressionReferenceChanged();
  void onTextEditChanged();
  void onLineEditChanged();
  void onLineEditFinished();
  void onInputValueChanged(QObject*);
  void doubleValueChanged(double newVal);
  void intValueChanged(int newVal);
  void updateItemData() override;
  // void setUseSelectionManager(bool mode) override;
  void showContextMenu(const QPoint& pt, int elementIdx);

protected Q_SLOTS:
  virtual void onAddNewValue();
  virtual void onRemoveValue();
  void displayExpressionWidget(bool checkstate);
  virtual void onChildItemModified();

protected:
  void createWidget() override;
  virtual void loadInputValues();
  virtual void updateUI();
  virtual void addInputEditor(int i);
  virtual void updateExtensibleState();
  virtual void clearChildWidgets();
  QWidget* createDoubleWidget(
    int elementIdx,
    smtk::attribute::ValueItemPtr vitem,
    QWidget* pWidget,
    QString& tooltip);
  QWidget* createIntWidget(
    int elementIdx,
    smtk::attribute::ValueItemPtr vitem,
    QWidget* pWidget,
    QString& tooltip);
  virtual QWidget* createInputWidget(int elementIdx, QLayout* childLayout);
  virtual QWidget* createEditBox(int elementIdx, QWidget* pWidget);
  virtual QFrame* createExpressionRefFrame();
  virtual QFrame* createLabelFrame(
    const smtk::attribute::ValueItem* vitem,
    const smtk::attribute::ValueItemDefinition* vitemDef);
  // Methods for updating widgets based on changes made to the underlying attribute item
  void updateDoubleItemData(QWidget* iwidget, const smtk::attribute::DoubleItemPtr& ditem);
  void updateIntItemData(QWidget* iwidget, const smtk::attribute::IntItemPtr& iitem);
  void updateStringItemData(QWidget* iwidget, const smtk::attribute::StringItemPtr& sitem);

  // Updates the item's expression reference widget. Takes the |elementIdx|th
  // value of |inputItem|. If |showMessageBox| == true, errors are presented
  // in a modal QMessageBox.
  void updateExpressionRefWidgetForEvaluation(
    smtk::attribute::ValueItemPtr inputItem,
    bool showMessageBox);
  void hideExpressionResultWidgets();
  // Shows widgets for displaying results of expression evaluation. |text| is
  // shown in a QLineEdit whose tooltip is set to |tooltip|. If |success| is
  // true but |tooltip| is nonempty, the line edit will have a dark yellow
  // background.
  void showExpressionResultWidgets(bool success, const QString& text, const QString& tooltip);

  /** \brief Determines (horizontal/vertical) orientation for values or children items. */
  Qt::Orientation getOrientation(const qtAttributeItemInfo& info);

private:
  qtInputsItemInternals* m_internals;
}; // class

//A sublcass of QDoubleValidator to fixup input outside of range
class SMTKQTEXT_EXPORT qtDoubleValidator : public QDoubleValidator
{
  Q_OBJECT
public:
  qtDoubleValidator(qtInputsItem* item, int elementIndex, QLineEdit* lineEdit, QObject* parent);
  void fixup(QString& input) const override;

private:
  qtInputsItem* m_item;
  int m_elementIndex;
  QLineEdit* m_lineWidget;
};

//A sublcass of QIntValidator to fixup input outside of range
class SMTKQTEXT_EXPORT qtIntValidator : public QIntValidator
{
  Q_OBJECT
public:
  qtIntValidator(qtInputsItem* item, int elementIndex, QLineEdit* lineEdit, QObject* parent);
  void fixup(QString& input) const override;

private:
  qtInputsItem* m_item;
  int m_elementIndex;
  QLineEdit* m_lineWidget;
};

}; // namespace extension
}; // namespace smtk

#endif
