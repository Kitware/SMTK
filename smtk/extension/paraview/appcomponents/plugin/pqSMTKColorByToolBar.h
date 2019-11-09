//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKColorByToolBar_h
#define smtk_extension_paraview_appcomponents_pqSMTKColorByToolBar_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

#include <QToolBar>

class pqServer;
class QAction;
class vtkSMSMTKResourceManagerProxy;

class pqSMTKColorByToolBar : public QToolBar
{
  Q_OBJECT
  using Superclass = QToolBar;

public:
  pqSMTKColorByToolBar(QWidget* parent = nullptr);
  ~pqSMTKColorByToolBar() override;

private:
  Q_DISABLE_COPY(pqSMTKColorByToolBar);
};

#endif
