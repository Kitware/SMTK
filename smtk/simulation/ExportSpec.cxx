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
  : m_logger(new smtk::io::Logger)
{
  this->clear();
}

void ExportSpec::clear()
{
  m_simulationResource = nullptr;
  m_exportResource = nullptr;
  m_logger->reset();
}

} // namespace simulation
} // namespace smtk
