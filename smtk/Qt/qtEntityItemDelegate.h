#ifndef __smtk_qt_QEntityItemDelegate_h
#define __smtk_qt_QEntityItemDelegate_h

#include "smtk/QtSMTKExports.h"

#include <QStyledItemDelegate>

namespace smtk {
  namespace model {

class QTSMTK_EXPORT QEntityItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  QEntityItemDelegate(QWidget* parent = 0);

  virtual QSize sizeHint(
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const;

  virtual void paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const;

  virtual QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex &index) const;

  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual void setModelData(
    QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex &index) const;

protected slots:
   virtual void commitAndCloseEditor();
};

  } // namespace model
} // namespace smtk

#endif // __smtk_qt_QEntityItemDelegate_h
