//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h

#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKResourceBrowser.h"

#include <QDockWidget>

class pqSMTKResourceBrowser;

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  */
class pqSMTKResourcePanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKResourcePanel(QWidget* parent = nullptr);
  ~pqSMTKResourcePanel() override;

protected:
  pqSMTKResourceBrowser* m_browser;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h
