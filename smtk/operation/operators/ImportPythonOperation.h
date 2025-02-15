//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_ImportPythonOperation_h
#define smtk_model_ImportPythonOperation_h

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{

/**\brief A class for adding python operations to the current session.

   Given a python file that describes an operation, this operation loads the
   python operation into the current session. The new operation is ready for use
   upon the successful completion of this operation (the session does not need
   to be restarted).
  */
class SMTKCORE_EXPORT ImportPythonOperation : public Operation
{
public:
  smtkTypeMacro(smtk::operation::ImportPythonOperation);
  smtkCreateMacro(ImportPythonOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::Operation);

  bool ableToOperate() override;

  static std::vector<std::string> importOperationsFromModule(const std::string&, Manager&);

  static bool importOperation(
    smtk::operation::Manager& manager,
    const std::string& moduleName,
    const std::string& opName);

protected:
  Result operateInternal() override;
  void generateSummary(Result&) override;
  Specification createSpecification() override;
};
} // namespace operation
} // namespace smtk

#endif // smtk_model_ImportPythonOperation_h
