//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_Dream3DPipelineOperator_h
#define __smtk_session_multiscale_Dream3DPipelineOperator_h

#include "smtk/bridge/multiscale/operators/PythonScriptOperator.h"
#include "vtkObject.h"

namespace smtk {
  namespace bridge {
    namespace multiscale {

/**\brief An operator for executing the AFRL Phase I Demo Dream3D Pipeline.
 *
 * For the AFRL Materials Phase I Demo, takes the DEFORM point tracking file,
 * DEFORM step file, the attribute array to use when enerating the zones, a list
 * of StatsGeneratorDataContainers for each of the zones, and the path to the
 * executable "PipelineRunner". With these data, the script outputs a DREAM3D
 * (xdmf) file contaning a 2-dimensional mesh & 3-dimensional grain boundary
 * profiles for each zone.
 */

class SMTKMULTISCALESESSION_EXPORT Dream3DPipelineOperator : public PythonScriptOperator
{
public:
  smtkTypeMacro(Dream3DPipelineOperator);
  smtkCreateMacro(Dream3DPipelineOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace multiscale
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_Dream3DPipelineOperator_h
