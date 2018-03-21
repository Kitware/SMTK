//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"

#include <QHBoxLayout>
#include <QLineEdit>

namespace smtk
{
namespace extension
{

qtDescriptivePhraseEditor::qtDescriptivePhraseEditor(QWidget* super)
  : QWidget(super)
{
  m_title = new QLineEdit(this);
  this->setFocusProxy(m_title);
  new QHBoxLayout(this);
  QObject::connect(m_title, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

qtDescriptivePhraseEditor::~qtDescriptivePhraseEditor()
{
}

/*
QSize qtDescriptivePhraseEditor::sizeHint() const
{
  return m_title->sizeHint();
}
*/

QString qtDescriptivePhraseEditor::title() const
{
  return m_title->text();
}

void qtDescriptivePhraseEditor::setTitle(const QString& text)
{
  m_title->setText(text);
}

} // namespace model
} // namespace smtk
