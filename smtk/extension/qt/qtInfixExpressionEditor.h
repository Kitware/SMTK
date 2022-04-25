//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtInfixExpressionEditor_h
#define smtk_extension_qtInfixExpressionEditor_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtAttributeItemInfo.h"
#include "smtk/extension/qt/qtInfixExpressionEditorRow.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/attribute/Evaluator.h"

#include <memory>

#include <QString>

class QLineEdit;

namespace smtk
{
namespace extension
{

class qtInfixExpressionEditorInternals;

// A qtItem ItemView for editing infix expressions. qtInfixExpressionEditor
// behaves similarly to qtInputsItem except its input is only expected to be
// a StringItem, because the user will be creating, modifying, and deleting
// infix expressions written as strings.
//
// Contains one or more qtInfixExpressionEditorRows.
class SMTKQTEXT_EXPORT qtInfixExpressionEditor : public qtItem
{
  Q_OBJECT

public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtInfixExpressionEditor(const qtAttributeItemInfo& info);
  ~qtInfixExpressionEditor() override;

  void setLabelVisible(bool) override;
  bool isFixedWidth() const override;

public Q_SLOTS:
  // Updates the UI elements to reconcile any differences with the underlying
  // StringItem.
  void updateItemData() override;
  // Updates the underlying StringItem with |text| at element |elementIdx|.
  void onInputValueChanged(const QString& text, int elementIdx);

protected:
  void createWidget() override;
  void loadInputValues();
  // Deletes tool buttons and editors when the StringItem is extensible.
  void clearChildWidgets();

  // Adds a qtInfixExpressionEditorRow for the StringItem's |i|th value.
  void addInputEditor(int i);
  // Refreshes this item's widget and calls loadInputValues().
  void updateUI();
  // Sets the enabled state of remove buttons when the StringItem is extensible
  // to match the StringItem's min and max number of allowed values.
  void updateExtensibleState();

  // Creates a new qtInfixExpressionEditorRow and sets its input line edit's
  // color.
  qtInfixExpressionEditorRow* createInputWidget(int elementIdx);

private:
  qtInfixExpressionEditorInternals* mp_internals;

private Q_SLOTS:
  // Adds a new value to the StringItem and calls addInputEditor().
  void onAddNewValue();
  // Removes the value corresponding to the minus button which sent the signal.
  // Then calls clearChildWidgets() and loadInputValues().
  void onRemoveValue();
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtInfixExpressionEditor_h
