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

#ifndef __smtk_extension_qtInputsItem_h
#define __smtk_extension_qtInputsItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QDoubleValidator>

class qtInputsItemInternals;
class QBoxLayout;
class QLayout;
class QLineEdit;

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
  static qtItem* createItemWidget(const AttributeItemInfo& info);
  qtInputsItem(const AttributeItemInfo& info);
  virtual ~qtInputsItem();
  void setLabelVisible(bool) override;
  void unsetValue(int elementIndex);
  bool setDiscreteValue(int elementIndex, int discreteValIndex);

public slots:
  void setOutputOptional(int);
  void onExpressionReferenceChanged();
  void onTextEditChanged();
  void onLineEditChanged();
  void onLineEditFinished();
  void onInputValueChanged(QObject*);
  void doubleValueChanged(double newVal);
  void intValueChanged(int newVal);
  // void setUseSelectionManager(bool mode) override;

protected slots:
  void updateItemData() override;
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
    int elementIdx, smtk::attribute::ValueItemPtr vitem, QWidget* pWidget, QString& tooltip);
  QWidget* createIntWidget(
    int elementIdx, smtk::attribute::ValueItemPtr vitem, QWidget* pWidget, QString& tooltip);
  virtual QWidget* createInputWidget(int elementIdx, QLayout* childLayout);
  virtual QWidget* createEditBox(int elementIdx, QWidget* pWidget);
  virtual QWidget* createExpressionRefWidget(int elementIdx);

private:
  qtInputsItemInternals* Internals;

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
