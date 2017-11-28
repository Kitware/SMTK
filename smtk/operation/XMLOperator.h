//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_XMLOperator_h
#define __smtk_operation_XMLOperator_h

#include "smtk/operation/NewOp.h"

namespace smtk
{
namespace operation
{

/// A specialization of smtk::operation::NewOp for operators whose
/// specifications are defined by an XML description that is accessible at
/// compile time.
class SMTKCORE_EXPORT XMLOperator : public smtk::operation::NewOp
{
public:
  smtkTypeMacro(XMLOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  virtual ~XMLOperator();

protected:
  XMLOperator();

  // Perform the actual operation and construct the result.
  virtual Result operateInternal() override = 0;

private:
  // Construct the operator's specification from the class's XML description.
  Specification createSpecification() override;

  // Access a block of text representing the XML description of the operator.
  virtual const char* xmlDescription() const = 0;
};
}
}

#endif // __smtk_operation_XMLOperator_h
