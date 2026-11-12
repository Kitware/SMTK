//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtRegexDelegate_h
#define smtk_extension_qtRegexDelegate_h

#include "smtk/extension/qt/Exports.h"

#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStyledItemDelegate>

namespace smtk
{
namespace extension
{
///\brief Simple delegate that uses a RegEx to restrict what can be entered

class SMTKQTEXT_EXPORT qtRegexDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  qtRegexDelegate(QObject* parent = nullptr)
    : QStyledItemDelegate(parent)
    , m_expression(".*")
  {
  }

  void setExpression(const std::string& exp) { m_expression.setPattern(exp.c_str()); }

protected:
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&)
    const override
  {
    QLineEdit* editor = new QLineEdit(parent);
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(m_expression, parent);
    editor->setValidator(validator);

    return editor;
  }
  QRegularExpression m_expression;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
