//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_SimulationAttribute_h
#define smtk_session_oscillator_SimulationAttribute_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/session/oscillator/Resource.h"

#include "smtk/attribute/Resource.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

class SMTKOSCILLATORSESSION_EXPORT SimulationAttribute
{
public:
  static smtk::attribute::ResourcePtr create();

  /**\brief Evaluate the simulation attribute and report actions a user
    *       should take to prepare it for export.
    *
    * Things this currently checks for:
    * + the simulation attribute must be linked to a geometric model;
    * + the model must have a single domain;
    * + the model must have at least one source sphere (an aux geom);
    * + each model source must be associated to an oscillator source-term attribute;
    * + the simulation attribute must pass its isValid() check.
    */
  bool lint(const smtk::attribute::ResourcePtr& simulation, smtk::io::Logger& feedback);

  smtk::session::oscillator::Resource::Ptr geometry() const { return m_geometry; }
  smtk::model::EntityPtr domain() const { return m_domain; }
  const smtk::attribute::Attributes& sourceTerms() const { return m_sources; }

protected:
  smtk::session::oscillator::Resource::Ptr m_geometry;
  smtk::model::EntityPtr m_domain;
  smtk::attribute::Attributes m_sources;
};

} // namespace oscillator
} // namespace session
} // namespace smtk

#endif // smtk_session_oscillator_SimulationAttribute_h
