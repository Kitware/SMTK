//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_operators_EditProperties_h
#define smtk_operation_operators_EditProperties_h

#include "smtk/operation/XMLOperation.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace operation
{

/// Set (or remove) an attribute value on a set of entities. The string, integer,
/// and floating-point values are all optional. Any combination may be specified.
/// All that are specified are set; those unspecified are removed.
///
/// Note that the current corresponding UI only allows for the creation of scalars,
/// so the SBT file does not have the string, int, and double items extensible, but the
/// operation itself will support them once the UI is modified.  All that needs to be
/// done is to add the extensibility property to those items.
class SMTKCORE_EXPORT EditProperties : public XMLOperation
{
public:
  smtkTypeMacro(smtk::operation::EditProperties);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace operation
} // namespace smtk

#endif
