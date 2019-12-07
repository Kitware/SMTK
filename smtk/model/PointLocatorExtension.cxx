//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/PointLocatorExtension.h"

#include <random>

namespace smtk
{
namespace model
{

PointLocatorExtension::PointLocatorExtension() = default;

PointLocatorExtension::~PointLocatorExtension() = default;

bool PointLocatorExtension::randomPoint(
  const EntityRef& entity, std::size_t nPoints, std::vector<double>& points)
{
  std::random_device rd;
  return this->randomPoint(entity, nPoints, points, rd());
}
}
}
