//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtNotEditableDelegate_h
#define smtk_extension_qtNotEditableDelegate_h

#include "smtk/extension/qt/Exports.h"

#include <QItemDelegate>

namespace smtk
{
namespace extension
{
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
  bool editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex&)
    override
  {
    return false;
  }
  QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override
  {
    return nullptr;
  }

}; // class
}; // namespace extension
}; // namespace smtk

#endif
