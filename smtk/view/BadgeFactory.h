//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_BadgeFactory_h
#define smtk_view_BadgeFactory_h

#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

#include <functional>
#include <map>
#include <string>

namespace smtk
{
namespace view
{
/**\brief A factory for badges that appear in descriptive phrases.
  *
  * A badge factory creates badges for use in PhraseModel views. A "badge"
  * in this context is an object that takes an input descriptive phrase and
  * returns an SVG icon with an optional tooltip and action to
  * be performed when clicked by a user. A badge may indicate it is not
  * applicable to a descriptive phrase.
  *
  * Badges are owned by a BadgeSet, which is in turn owned by a PhraseModel,
  * Thus, there is a single Badge instance per PhraseModel rather than one
  * per DescriptivePhrase.
  * To register a Badge subclass, it must have a default constructor
  * and a constructor that takes references to a BadgeSet and a
  * Configuration::Component.
  */
class SMTKCORE_EXPORT BadgeFactory
  : public smtk::common::Factory<
      Badge,
      void,
      smtk::common::factory::Inputs<BadgeSet&, const Configuration::Component&>>
{
};
} // namespace view
} // namespace smtk

#endif
