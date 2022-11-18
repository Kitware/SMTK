//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_DumpGraph_h
#define smtk_markup_DumpGraph_h

#include "smtk/markup/Resource.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/// Dump a graph of the nodes in a markup resource in graphviz format.
class SMTKMARKUP_EXPORT DumpGraph : public smtk::operation::XMLOperation
{

public:
  smtkTypeMacro(smtk::markup::DumpGraph);
  smtkCreateMacro(DumpGraph);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_DumpGraph_h
