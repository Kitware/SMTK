//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_Resource_h
#define smtk_session_oscillator_Resource_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/session/oscillator/Exports.h"

#include "smtk/resource/DerivedFrom.h"

#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

class SMTKOSCILLATORSESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::model::Resource>
{
public:
  smtkTypeMacro(smtk::session::oscillator::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  ~Resource() override = default;

  bool resetDomainTessellation(smtk::model::Volume& v);

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);
};

} // namespace oscillator
} // namespace session
} // namespace smtk

#endif // smtk_session_oscillator_Resource_h
