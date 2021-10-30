//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_model_utility_InterpolateField_h
#define smtk_model_utility_InterpolateField_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <string>
#include <utility>
#include <vector>

namespace smtk
{
namespace attribute
{
class Attribute;
class Definition;
} // namespace attribute

namespace model
{

/// Weights are a vector of distances and associated attributes. The distances
/// are unnormalized and have whatever units the coordinates have.
typedef std::vector<std::pair<double, const smtk::attribute::Attribute* const>> Weights;

/// Compute weights for a each sample point in <samplePoints> according to
/// geometry associated with <definition>.
SMTKCORE_EXPORT
std::vector<Weights> computeWeights(
  const std::vector<double>& samplePoints,
  const smtk::attribute::DefinitionPtr& definition);

/// Use inverse distance weighting to interpolate values held by the double item
/// named <itemName>.
SMTKCORE_EXPORT
std::vector<double> inverseDistanceWeighting(
  const std::vector<Weights>& weightsForPoints,
  const std::string& itemName,
  const double& power);
} // namespace model
} // namespace smtk

#endif
