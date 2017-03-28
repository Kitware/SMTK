//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_PythonScriptOperator_h
#define __smtk_session_multiscale_PythonScriptOperator_h

#include "smtk/bridge/multiscale/Operator.h"

namespace smtk {
  namespace bridge {
    namespace multiscale {

/**\brief An operator for executing python scripts as standalone executables.
 *
 * This operator is a wrapper for executing standalone python scripts that are
 * controlled using python's argparse module.
 */

class SMTKMULTISCALESESSION_EXPORT PythonScriptOperator : public Operator
{
protected:
  // Convert a vector of arguments into a formatted argument list
  virtual std::string listToArgList(std::vector<std::string>& tokens);

  // Convert an attribute spec into a formatted argument list
  virtual std::string specToArgList(smtk::attribute::AttributePtr spec);

  // Convert an attribute spec and an additional vector of arguements into a
  // formatted list
  virtual std::string specToArgList(smtk::attribute::AttributePtr spec,
                                    std::vector<std::string>& additionalTokens);

  // Execute the python script using the vtkPythonInterpreter
  virtual smtk::model::OperatorResult
    executePythonScript(std::string preamble, std::string pythonScript);
};

    } // namespace multiscale
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_PythonScriptOperator_h
