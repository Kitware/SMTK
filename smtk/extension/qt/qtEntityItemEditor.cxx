#include "smtk/extension/qt/qtEntityItemEditor.h"

#include <QtGui/QLineEdit>
#include <QtGui/QHBoxLayout>

namespace smtk {
  namespace model {

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
