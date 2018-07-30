//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCumulusJobsPanel - display jobs on remote system
// .SECTION Description

#ifndef __CmbJobsPanels_h
#define __CmbJobsPanels_h

#include "smtk/extension/cumulus/Exports.h"

#include <QDockWidget>

class SMTKCUMULUSEXT_EXPORT pqCumulusJobsPanel : public QDockWidget
{
  Q_OBJECT

public:
  pqCumulusJobsPanel(QWidget* parent);
  virtual ~pqCumulusJobsPanel();

signals:
  void loadSimulationResults(const QString& path);
  void resultDownloaded(const QString& path);

public slots:

protected slots:
  void infoSlot(const QString& msg);
  void authenticateHPC();
  void requestSimulationResults();

private:
  class pqCumulusJobsPanelInternal;
  pqCumulusJobsPanelInternal* Internal;
};

#endif // __CmbJobsPanels_h
