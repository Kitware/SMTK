//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_XMLOperation_h
#define __smtk_operation_XMLOperation_h

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{

/// A specialization of smtk::operation::Operation for operations whose
/// specifications are defined by an XML description that is accessible at
/// compile time.
class SMTKCORE_EXPORT XMLOperation : public smtk::operation::Operation
{
public:
  smtkTypeMacro(XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  ~XMLOperation() override;

protected:
  XMLOperation();

  // Perform the actual operation and construct the result.
  Result operateInternal() override = 0;

  // Construct the operation's specification from the class's XML description.
  Specification createSpecification() override;

private:
  // Access a block of text representing the XML description of the operation.
  virtual const char* xmlDescription() const = 0;
};
} // namespace operation
} // namespace smtk

#endif // __smtk_operation_XMLOperation_h
