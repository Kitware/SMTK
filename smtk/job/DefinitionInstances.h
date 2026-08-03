//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_DefinitionInstances_h
#define smtk_job_DefinitionInstances_h

#include "smtk/job/Definition.h"

#include "smtk/common/Instances.h"

namespace smtk
{
namespace job
{

/// A class that manages instances of job::Definition objects.
class SMTKCORE_EXPORT DefinitionInstances : public smtk::common::Instances<smtk::job::Definition>
{
public:
  smtkTypeMacroBase(smtk::job::DefinitionInstances);
  smtkCreateMacro(smtk::job::DefinitionInstances);

  DefinitionInstances() = default;
  virtual ~DefinitionInstances() = default;

  std::shared_ptr<Definition> findByName(const std::string& name);
};

} // namespace job
} // namespace smtk

#endif // smtk_job_DefinitionInstances_h
