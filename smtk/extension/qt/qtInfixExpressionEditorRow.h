//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtInfixExpressionEditorRow_h
#define smtk_extension_qtInfixExpressionEditorRow_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/attribute/Evaluator.h"

#include <memory>

#include <QEvent>
#include <QObject>
#include <QString>
#include <QWidget>

class QLineEdit;

namespace smtk
{
namespace extension
{

// A widget for editing a single infix expression. The user types expressions
// in a line edit on the left, while a read-only line edit on the right displays
// the result or error in evaluations.
class SMTKQTEXT_EXPORT qtInfixExpressionEditorRow : public QWidget
{
  Q_OBJECT

public:
  // |elementIdx| is the index in the underlying StringItem.
  // |mp_editBox| will be initialized displaying |text|.
  qtInfixExpressionEditorRow(
    const QString& text,
    int elementIdx,
    std::unique_ptr<smtk::attribute::Evaluator> evaluator,
    QWidget* parent);

  // Adds context menu action for onShowExpressionHelp() if |ev| is a
  // ContextMenu event, else passes |ev| to QWidget::eventFilter().
  bool eventFilter(QObject* filterObj, QEvent* ev) override;

  QLineEdit* editBox() const;
  int itemElementIndex() const;

public Q_SLOTS:
  // Evaluates the result of |text| and displays it in |mp_resultBox|.
  void onEditBoxChanged(const QString& text);

Q_SIGNALS:
  // Emitted from onEditBoxChanged().
  void editBoxChanged(const QString& text, int elementIndex);

private:
  // The line edit which the user types in.
  QLineEdit* mp_editBox;
  // The read-only line edit which displays results from |mp_editBox|.
  QLineEdit* mp_resultBox;
  // The index in the underlying StringItem for this widget.
  int m_elementIdx;
  // The Evaluator we will use for this expression.
  std::unique_ptr<smtk::attribute::Evaluator> mp_evaluator;

  // Sets the QPalette::base color of |mp_resultBox| with proper contrast.
  void setResultBoxColor(bool resultIsValid);

private Q_SLOTS:
  // Shows a dialog containing instructions and reference on using infix
  // expressions.
  void onShowExpressionHelp();
};

} // namespace extension
} // namespace smtk

#endif // __qtInfixExpressionEditorRow_h
