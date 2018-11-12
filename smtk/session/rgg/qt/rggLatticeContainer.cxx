//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/qt/rggLatticeContainer.h"

#include <limits>

rggLatticeContainer::rggLatticeContainer(smtk::model::EntityRef entity)
  : m_entity(entity)
{
}

rggLatticeContainer::~rggLatticeContainer()
{
}

// latice container functions
std::pair<size_t, size_t> rggLatticeContainer::GetDimensions()
{
  return this->m_lattice.GetDimensions();
}

void rggLatticeContainer::updateLaticeFunction()
{
  double ri, rj;
  this->getRadius(ri, rj);
  m_lattice.updatePitchForMaxRadius(this->getPitchX(), this->getPitchY());
  m_lattice.updateMaxRadius(ri, rj);
  m_lattice.sendMaxRadiusToReference();
}

void rggLatticeContainer::getRadius(double& ri, double& rj) const
{
  ri = std::numeric_limits<double>::max();
  rj = std::numeric_limits<double>::max();
}
