//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_SubphraseGeneratorFactory_h
#define smtk_view_SubphraseGeneratorFactory_h

#include "smtk/CoreExports.h"
#include "smtk/common/Factory.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

/**\brief A factory to create suphrase generators which is held by view managers
*/
class SMTKCORE_EXPORT SubphraseGeneratorFactory
  : public smtk::common::Factory<SubphraseGenerator, void>
{
public:
  std::unique_ptr<SubphraseGenerator> createFromConfiguration(
    const Configuration::Component* config)
  {
    if (!config)
    {
      return std::unique_ptr<SubphraseGenerator>();
    }
    std::string sgType;
    bool haveType = config->attribute("Type", sgType);
    if (!haveType || sgType.empty() || sgType == "default")
    {
      sgType = smtk::common::typeName<SubphraseGenerator>();
    }
    return this->createFromName(sgType);
  }
};
} // namespace view
} // namespace smtk

#endif
