//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_qt_qtCheckItemComboBox_h
#define __smtk_qt_qtCheckItemComboBox_h

#include "smtk/extension/qt/QtSMTKExports.h"
#include <QStyledItemDelegate>
#include <QComboBox>

class QStandardItem;

namespace smtk {
  namespace attribute {

//A sublcass of QTextEdit to give initial sizehint
class QTSMTK_EXPORT qtCheckableComboItemDelegate : public QStyledItemDelegate
  {
  Q_OBJECT
  public:
    qtCheckableComboItemDelegate(QWidget * owner);
    virtual void paint(
      QPainter* painter,
      const QStyleOptionViewItem& option,
      const QModelIndex& index) const;
  };

//A sublcass of QComboBox to set text when hidePopup
class QTSMTK_EXPORT qtCheckItemComboBox : public QComboBox
  {
  Q_OBJECT
  public:
    qtCheckItemComboBox(QWidget * parentW, const QString& displayExt);
    virtual void hidePopup();
    virtual void init();
    virtual void updateText();

  private:
    QStandardItem* m_displayItem;
    QString m_displayTextExt;
  };

  } // namespace attribute
} // namespace smtk

#endif // __smtk_qt_qtCheckItemComboBox_h
