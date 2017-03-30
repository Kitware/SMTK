//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ExportSpec.h"

namespace smtk
{
namespace simulation
{

ExportSpec::ExportSpec()
{
  this->clear();
}

void ExportSpec::clear()
{
  m_simulationSystem = NULL;
  m_exportSystem = NULL;
  m_analysisGridInfo = smtk::model::GridInfoPtr();
  m_logger.reset();
}

} // namespace simulation
} // namespace smtk
