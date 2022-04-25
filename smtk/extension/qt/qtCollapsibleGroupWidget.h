//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCollapsibleGroupWidget -Widget that can be "collapsed/closed"
// .SECTION Description
// .SECTION See Also

#ifndef smtk_extension_qtCollapsibleGroupWidget_h
#define smtk_extension_qtCollapsibleGroupWidget_h

#include "smtk/extension/qt/Exports.h"
#include <QWidget>

class QFrame;
namespace smtk
{
namespace extension
{
class qtCollapsibleGroupWidgetInternals;

class SMTKQTEXT_EXPORT qtCollapsibleGroupWidget : public QWidget
{
  Q_OBJECT

public:
  qtCollapsibleGroupWidget(QWidget* parent);
  ~qtCollapsibleGroupWidget() override;

  QLayout* contentsLayout() const;
  void setContentsLayout(QLayout* newLayout);
  QFrame* contents() const;
  void setName(const QString& newName);
  QString name() const;

public Q_SLOTS:
  void open();
  void collapse();

protected:
  qtCollapsibleGroupWidgetInternals* m_internals;

private:
};
}; // namespace extension
}; // namespace smtk

#endif /* smtk_extension_qtCollapsibleGroupWidget_h */
