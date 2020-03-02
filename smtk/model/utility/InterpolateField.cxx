//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/utility/InterpolateField.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/PointLocatorExtension.h"

#include <array>
#include <cmath>

namespace
{
const double EPSILON = 1.e-10;
}

namespace smtk
{
namespace model
{

std::vector<Weights> computeWeights(
  const std::vector<double>& samplePoints, const smtk::attribute::DefinitionPtr& definition)
{
  // Access the attribute resource containing the annotations
  smtk::attribute::ResourcePtr attributeResource =
    definition != nullptr ? definition->resource() : nullptr;

  // Construct a vector of point profiles
  std::vector<Weights> pointProfiles(samplePoints.size() / 3);

  // Construct a container to hold the nearest points (passed into the point
  // locator)
  std::vector<double> interpolatedPoints(samplePoints.size());

  // Access the attributes associated with the input definition
  std::vector<smtk::attribute::AttributePtr> attributes;
  if (attributeResource != nullptr)
  {
    attributeResource->findAttributes(definition, attributes);
  }

  for (const auto& attribute : attributes)
  {
    // Access the attribute's associations
    smtk::attribute::ReferenceItem::ConstPtr referenceItem = attribute->associations();
    for (std::size_t i = 0; i < referenceItem->numberOfValues(); ++i)
    {
      if (!referenceItem->isSet(i))
      {
        continue;
      }

      // Cast the association to a model EntityRef
      smtk::model::EntityRef entityRef(referenceItem->valueAs<smtk::model::Entity>(i));
      if (!entityRef.isValid())
      {
        continue;
      }

      bool computed = false;

      // Visit all extensions, attempting to cast them to a point locator
      smtk::common::Extension::visitAll([&](
        const std::string&, smtk::common::Extension::Ptr extension) {
        bool success = false;
        auto pointLocator =
          smtk::dynamic_pointer_cast<smtk::model::PointLocatorExtension>(extension);

        if (pointLocator != nullptr)
        {
          // If we find a point locator, compute the closest point on the
          // entity for each of our sample points
          success = pointLocator->closestPointOn(entityRef, interpolatedPoints, samplePoints, true);
          computed = success;
        }
        return std::make_pair(success, success);
      });

      // If the point locator call was successful...
      if (computed)
      {
        // ...convert each computed point into a distance weighting and pair it
        // with a pointer to the attribute.
        std::size_t counter = 0;
        for (std::size_t j = 0; j < samplePoints.size(); j += 3)
        {
          double dist = 0;
          for (std::size_t k = 0; k < 3; k++)
          {
            double tmp = interpolatedPoints[j + k] - samplePoints[j + k];
            dist += tmp * tmp;
          }
          dist = std::sqrt(dist);
          pointProfiles[counter++].push_back(std::make_pair(dist, attribute.get()));
        }
      }
    }
  }

  return pointProfiles;
}

std::vector<double> inverseDistanceWeighting(
  const std::vector<Weights>& weightsForPoints, const std::string& itemName, const double& power)
{
  // Initialize the output vector
  std::vector<double> values(weightsForPoints.size());

  std::size_t pointIndex = 0;

  // For each point...
  for (auto& weightsForPoint : weightsForPoints)
  {
    double w = 0., num = 0., denom = 0.;
    // ...for each weight...
    for (auto& weight : weightsForPoint)
    {
      if (weight.second == nullptr)
      {
        continue;
      }

      const smtk::attribute::DoubleItem* doubleItem = weight.second->findDouble(itemName).get();
      if (doubleItem == nullptr)
      {
        continue;
      }

      // ...if the sample point is on top of a geometry with an assigned value,
      // forego the interpolation and assign the value directly to the point.
      if (weight.first < EPSILON)
      {
        values[pointIndex++] = doubleItem->value();
        break;
      }

      // Otherwise, weight the value of the term by an inverse power if its
      // distance to the geometry with this value.
      w = std::pow(weight.first, -1. * power);
      num += w * doubleItem->value();
      denom += w;
    }
    values[pointIndex++] = (denom > 0. ? num / denom : std::numeric_limits<double>::quiet_NaN());
  }
  return values;
}
}
}
