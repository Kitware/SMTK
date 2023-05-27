//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtCheckItemComboBox_h
#define smtk_extension_qtCheckItemComboBox_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"

#include <QComboBox>
#include <QModelIndex>
#include <QStyledItemDelegate>

class QStandardItem;

namespace smtk
{
namespace extension
{

//A sublcass of QTextEdit to give initial sizehint
class SMTKQTEXT_EXPORT qtCheckableComboItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  qtCheckableComboItemDelegate(QWidget* owner);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
    const override;
};

//A sublcass of QComboBox to set text when hidePopup
class SMTKQTEXT_EXPORT qtCheckItemComboBox : public QComboBox
{
  Q_OBJECT
public:
  qtCheckItemComboBox(QWidget* parentW, const QString& displayExt);
  void hidePopup() override;
  void showPopup() override;
  virtual void init();
  virtual void updateText();

protected:
  bool eventFilter(QObject* editor, QEvent* event) override;
  void showEvent(QShowEvent* e) override;

private:
  QStandardItem* m_displayItem{ nullptr };
  QString m_displayTextExt;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtCheckItemComboBox_h
