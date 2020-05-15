//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtNotEditableDelegate_h
#define __smtk_extension_qtNotEditableDelegate_h

#include "smtk/extension/qt/Exports.h"

#include <QItemDelegate>

namespace smtk
{
namespace extension
{
class qtAssociationWidget;
class qtBaseView;

///\brief Simple delegate that prevents contents from being modified

class SMTKQTEXT_EXPORT qtNotEditableDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  qtNotEditableDelegate(QObject* parent = nullptr)
    : QItemDelegate(parent)
  {
  }

protected:
  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
    const QModelIndex& index)
  {
    return false;
  }
  QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const
  {
    return nullptr;
  }

}; // class
}; // namespace extension
}; // namespace smtk

#endif
