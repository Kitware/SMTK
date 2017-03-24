//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME jobview.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_jobview_h
#define __smtk_extension_cumulus_jobview_h

#include "job.h"

#include <QTableView>
#include <QSet>


namespace cumulus
{
class CumulusProxy;

class JobView : public QTableView
{
  Q_OBJECT

public:
  JobView(QWidget *theParent = 0);
  ~JobView();

  void contextMenuEvent(QContextMenuEvent *e);
  void setCumulusProxy(CumulusProxy *cumulusProxy);

private slots:
  void deleteJob();
  void terminateJob();
  void downloadJob();

private:
  CumulusProxy *m_cumulusProxy;

};

} // end of namespace

#endif
