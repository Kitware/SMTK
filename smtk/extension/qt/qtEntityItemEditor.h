#ifndef __smtk_qt_QEntityItemEditor_h
#define __smtk_qt_QEntityItemEditor_h

#include "smtk/extension/qt/QtSMTKExports.h"
#include <QtGui/QWidget>

class QLineEdit;

namespace smtk {
  namespace model {

/**\brief Allow user edits to an smtk::model::Manager instance via QEntityItemModel.
  *
  */
class QTSMTK_EXPORT QEntityItemEditor : public QWidget
{
  Q_OBJECT
public:
  QEntityItemEditor(QWidget* parent = 0);
  virtual ~QEntityItemEditor();

  //QSize sizeHint() const;
  QString title() const;
  void setTitle(const QString& text);

signals:
  void editingFinished();

protected:
  QLineEdit* m_title;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_qt_QEntityItemEditor_h
