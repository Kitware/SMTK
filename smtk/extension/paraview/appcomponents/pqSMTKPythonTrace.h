//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqPythonTrace_h
#define smtk_extension_paraview_appcomponents_pqPythonTrace_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include <string>

/**\brief Utility for adding operations to the python trace.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKPythonTrace
{

public:
  pqSMTKPythonTrace() = default;
  ~pqSMTKPythonTrace() = default;

  std::string traceOperation(const smtk::operation::Operation& op);

private:
  bool m_showSetup = true;
};

#endif
