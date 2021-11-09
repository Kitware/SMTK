//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ExportSpec - Application data for passing to python scripts.
// .SECTION Description
// This class if for storing application data passed from application code
// to python scripts.
// .SECTION See Also

#ifndef smtk_simulation_ExportSpec_h
#define smtk_simulation_ExportSpec_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/Logger.h"

#include <string>
#include <vector>

namespace smtk
{
namespace simulation
{
class SMTKCORE_EXPORT ExportSpec
{
public:
  // Data-get methods, intended to be called from python scripts
  smtk::attribute::ResourcePtr getSimulationAttributes() const { return m_simulationResource; }
  smtk::attribute::ResourcePtr getExportAttributes() const { return m_exportResource; }
  smtk::io::LoggerPtr getLogger() const { return m_logger; }

  // Constructor and data-set methods, intended to be called from C/C++ code
  ExportSpec();
  void clear();

  void setSimulationAttributes(smtk::attribute::ResourcePtr resource)
  {
    m_simulationResource = resource;
  }
  void setExportAttributes(smtk::attribute::ResourcePtr resource) { m_exportResource = resource; }

private:
  smtk::attribute::ResourcePtr m_simulationResource;
  smtk::attribute::ResourcePtr m_exportResource;
  smtk::io::LoggerPtr m_logger;
};
} // namespace simulation
} // namespace smtk

#endif /* smtk_simulation_ExportSpec_h */
