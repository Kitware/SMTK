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

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

#include <QToolBar>

class pqServer;
class QAction;
class vtkSMSMTKResourceManagerProxy;

/**\brief A toolbar that lets users manipulate how a resource's representation is colored.
  *
  * When users select a resource in the resource panel (or take other
  * actions that cause ParaView to select the pipeline object exposing
  * a resource), this toolbar presents options for how the representation
  * is colored in the active view.
  *
  * Currently, representations may be colored "by entity" (in which case
  * colors explicitly assigned to each component are used if available),
  * "by volume" (in which case, surface are rendered in the color assigned
  * to the region of space they bound), or "by field" (in which case
  * spatially-varying point- or cell-data is applied to cells via a colormap).
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKColorByToolBar : public QToolBar
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
