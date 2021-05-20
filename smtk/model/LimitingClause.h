//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_LimitingClause_h
#define __smtk_model_LimitingClause_h

#include "smtk/CoreExports.h" // for SMTKCORE_EXPORT macro

#include "smtk/resource/PropertyType.h"

#include <string>
#include <vector>

namespace smtk
{
namespace model
{

/**!\brief Parser state for model-entity filter specifications.
  *
  * This object is created by smtk::model::Entity::filterStringToQueryFunctor()
  * and used by the returned functor to evaluate entities for suitability in
  * query results.
  */
struct SMTKCORE_EXPORT LimitingClause
{
  LimitingClause() = default;

  smtk::resource::PropertyType m_propType{ smtk::resource::PropertyType::INVALID_PROPERTY };
  std::string m_propName;
  bool m_propNameIsRegex{ false };
  std::vector<std::string> m_propStringValues;
  std::vector<bool> m_propStringIsRegex;
  std::vector<long> m_propIntValues;
  std::vector<double> m_propFloatValues;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_LimitingClause_h
