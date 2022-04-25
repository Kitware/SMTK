//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtInfixExpressionEditorRow.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/io/Logger.h"

#include <string>

#include <QColor>
#include <QContextMenuEvent>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPalette>
#include <QSizePolicy>

// clang-format off

const std::string expressionHelpText =
R"***(
<html>
<body>
  <div>Use standard mathematical operations: +, -, *, / and ^ for exponentation.</div>
  <br/>
  <div>Use an an expression written elsewhere by writing its name in braces "{}". E.g: 12 + {abc}</div>
  <br/>
  <div>Mathematical functions:</div>
  <table>
    <tr>
      <th>Function</th><th>Syntax</th>
    </tr>
    <tr>
      <td>sine</td><td>sin(x)</td>
    </tr>
    <tr>
      <td>cosine</td><td>cos(x)</td>
    </tr>
    <tr>
      <td>tangent</td><td>tan(x)</td>
    </tr>
    <tr>
      <td>inverse sine</td><td>asin(x)</td>
    </tr>
    <tr>
      <td>inverse cosine</td><td>acos(x)</td>
    </tr>
    <tr>
      <td>inverse tangent</td><td>atan(x)</td>
    </tr>
    <tr>
      <td>hyperbolic sine</td><td>sinh(x)</td>
    </tr>
    <tr>
      <td>hyperbolic cosine</td><td>cosh(x)</td>
    </tr>
    <tr>
      <td>hyperbolic tangent</td><td>tanh(x)</td>
    </tr>
    <tr>
      <td>inverse hyperbolic sine</td><td>asinh(x)</td>
    </tr>
    <tr>
      <td>inverse hyperbolic cosine</td><td>acosh(x)</td>
    </tr>
    <tr>
      <td>inverse hyperbolic tangent</td><td>atanh(x)</td>
    </tr>
    <tr>
      <td>logarithm base e</td><td>ln(x)</td>
    </tr>
    <tr>
      <td>logarithm base 10</td><td>log10(x)</td>
    </tr>
    <tr>
      <td>exponential e^x</td><td>exp(x)</td>
    </tr>
    <tr>
      <td>square root</td><td>sqtr(x)</td>
    </tr>
  </table>
</body>
</html>
)***";

// clang-format on

using namespace smtk::attribute;

namespace smtk
{
namespace extension
{

qtInfixExpressionEditorRow::qtInfixExpressionEditorRow(
  const QString& text,
  int elementIdx,
  std::unique_ptr<Evaluator> evaluator,
  QWidget* parent)
  : QWidget(parent)
  , m_elementIdx(elementIdx)
  , mp_evaluator(std::move(evaluator))
{
  mp_editBox = new QLineEdit(this);
  mp_editBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  mp_editBox->setText(text);

  connect(mp_editBox, &QLineEdit::textChanged, this, &qtInfixExpressionEditorRow::onEditBoxChanged);

  mp_resultBox = new QLineEdit(this);
  mp_resultBox->setReadOnly(true);

  QHBoxLayout* layout = new QHBoxLayout(this);

  setLayout(layout);
  layout->addWidget(mp_editBox);
  layout->addWidget(new QLabel("=", this));
  layout->addWidget(mp_resultBox);

  // Manually triggers computation of the result.
  onEditBoxChanged(text);

  mp_editBox->installEventFilter(this);
}

bool qtInfixExpressionEditorRow::eventFilter(QObject* filterObj, QEvent* ev)
{
  if (ev->type() == QEvent::ContextMenu)
  {
    QContextMenuEvent* menuEvent = static_cast<QContextMenuEvent*>(ev);

    QMenu* contextMenu = editBox()->createStandardContextMenu();
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addSeparator();
    contextMenu->addAction(
      "Expression Help", this, &qtInfixExpressionEditorRow::onShowExpressionHelp);

    contextMenu->popup(menuEvent->globalPos());

    ev->accept();
    return true;
  }
  else
  {
    return QWidget::eventFilter(filterObj, ev);
  }
}

QLineEdit* qtInfixExpressionEditorRow::editBox() const
{
  return mp_editBox;
}

int qtInfixExpressionEditorRow::itemElementIndex() const
{
  return m_elementIdx;
}

void qtInfixExpressionEditorRow::onEditBoxChanged(const QString& text)
{
  Q_EMIT this->editBoxChanged(text, itemElementIndex());

  if (text.isEmpty())
  {
    mp_resultBox->setText(QString());
    mp_resultBox->setToolTip(QString());
    setResultBoxColor(true);
  }
  else
  {
    smtk::io::Logger log;
    Evaluator::ValueType result;

    if (mp_evaluator)
    {
      mp_evaluator->evaluate(
        result,
        log,
        static_cast<std::size_t>(itemElementIndex()),
        Evaluator::DependentEvaluationMode::DO_NOT_EVALUATE_DEPENDENTS);
    }
    else
    {
      log.addRecord(smtk::io::Logger::Error, "Invalid evaluator");
    }

    if (log.hasErrors())
    {
      std::vector<smtk::io::Logger::Record> recs = log.records();
      QString message;
      for (auto it = recs.begin(); it != recs.end(); ++it)
      {
        message += it->message.c_str();
        if (it != --recs.end())
        {
          message += "\n";
        }
      }

      mp_resultBox->setText("Evaluation failed");
      mp_resultBox->setToolTip(message);
      setResultBoxColor(false);
    }
    else
    {
      double computation;
      try
      {
        computation = boost::get<double>(result);
      }
      catch (const boost::bad_get&)
      {
        // We should not get here because this is the infix expression widget!
        mp_resultBox->setText("Evaluation failed");
        mp_resultBox->setToolTip("Result type was incompatible");
        setResultBoxColor(false);
        return;
      }

      mp_resultBox->setText(QString::number(computation));
      mp_resultBox->setToolTip(QString());
      setResultBoxColor(true);
    }
  }
}

void qtInfixExpressionEditorRow::setResultBoxColor(bool resultIsValid)
{
  QPalette pal = mp_resultBox->palette();

  QColor baseColor;
  if (resultIsValid)
  {
    baseColor = qtUIManager::contrastWithText(Qt::white);
  }
  else
  {
    QColor invalidColor;
    invalidColor.setRgbF(1.0, 0.5, 0.5);
    baseColor = qtUIManager::contrastWithText(invalidColor);
  }

  pal.setColor(QPalette::Base, baseColor);
  mp_resultBox->setPalette(pal);
}

void qtInfixExpressionEditorRow::onShowExpressionHelp()
{
  QDialog* expressionHelpDialog = new QDialog(this);
  // This dialog is not modal, so it can't be on the stack.
  expressionHelpDialog->setAttribute(Qt::WA_DeleteOnClose);
  expressionHelpDialog->setWindowTitle("Expression Help");

  QVBoxLayout* layout = new QVBoxLayout(expressionHelpDialog);
  // TODO: make the text in this label look nicer.
  layout->addWidget(new QLabel(expressionHelpText.c_str(), expressionHelpDialog));

  expressionHelpDialog->setLayout(layout);
  expressionHelpDialog->show();
}

} // namespace extension
} // namespace smtk
