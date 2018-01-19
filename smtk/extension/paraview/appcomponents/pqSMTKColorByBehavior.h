//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKColorByBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKColorByBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

#include <QActionGroup>

class pqServer;
class QToolBar;
class vtkSMSMTKResourceManagerProxy;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKColorByBehavior : public QActionGroup
{
  Q_OBJECT
  using Superclass = QActionGroup;

public:
  pqSMTKColorByBehavior(QObject* parent = nullptr);
  ~pqSMTKColorByBehavior() override;

  static pqSMTKColorByBehavior* instance();

public slots:
  virtual void customizeToolBar(QToolBar* tb);

protected slots:
  virtual void onFilterChanged(QAction* a);

protected:
  class pqInternal;
  pqInternal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKColorByBehavior);
};

#endif
