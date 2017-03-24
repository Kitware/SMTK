//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtEntityItemEditor.h"

#include <QLineEdit>
#include <QHBoxLayout>

namespace smtk {
  namespace extension {

QEntityItemEditor::QEntityItemEditor(QWidget* super)
  : QWidget(super)
{
  this->m_title = new QLineEdit(this);
  this->setFocusProxy(this->m_title);
  new QHBoxLayout(this);
  QObject::connect(
    this->m_title, SIGNAL(editingFinished()),
    this, SIGNAL(editingFinished()));
}

QEntityItemEditor::~QEntityItemEditor()
{
}

/*
QSize QEntityItemEditor::sizeHint() const
{
  return this->m_title->sizeHint();
}
*/

QString QEntityItemEditor::title() const
{
  return this->m_title->text();
}

void QEntityItemEditor::setTitle(const QString& text)
{
  this->m_title->setText(text);
}

  } // namespace model
} // namespace smtk
