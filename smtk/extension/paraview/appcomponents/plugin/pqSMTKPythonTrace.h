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

#include "smtk/PublicPointerDefs.h"

/**\brief Utility for adding operations to the python trace.
  *
  */
class pqSMTKPythonTrace
{

public:
  pqSMTKPythonTrace() = default;
  ~pqSMTKPythonTrace() = default;

  void traceOperation(const smtk::operation::Operation& op);

private:
  bool m_showSetup = true;
};

#endif
