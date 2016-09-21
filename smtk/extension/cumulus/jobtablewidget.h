//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME jobtablewidget.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_jobtablewidget_h
#define __smtk_extension_cumulus_jobtablewidget_h

#include <QtGui/QWidget>
class QAbstractItemModel;

namespace Ui {
class JobTableWidget;
}

namespace cumulus
{
class JobTableModel;
class CumulusProxy;

class JobTableWidget : public QWidget
{
  Q_OBJECT

public:
  explicit JobTableWidget(QWidget *parentObject = 0);
  ~JobTableWidget();

  void setModel(QAbstractItemModel *model);
  void setCumulusProxy(CumulusProxy *cumulusProxy);
protected:
  Ui::JobTableWidget *ui;

};

} // end namespace

#endif
