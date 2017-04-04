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


#ifndef __smtk_simulation_ExportSpec_h
#define __smtk_simulation_ExportSpec_h

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
      smtk::attribute::System *getSimulationAttributes() const
      { return m_simulationSystem; }
      smtk::attribute::System *getExportAttributes() const
      { return m_exportSystem; }
      smtk::model::GridInfoPtr getAnalysisGridInfo() const
      { return m_analysisGridInfo; }
      smtk::io::Logger getLogger() const
      { return m_logger; }

      // Constructor and data-set methods, intended to be called from C/C++ code
      ExportSpec();
      void clear();

      void setSimulationAttributes(smtk::attribute::System *system)
      { m_simulationSystem = system; }
      void setExportAttributes(smtk::attribute::System *system)
      { m_exportSystem = system; }
      void setAnalysisGridInfo(smtk::model::GridInfoPtr analysisGridInfo)
      { m_analysisGridInfo = analysisGridInfo; }

    private:
      smtk::attribute::System *m_simulationSystem;
      smtk::attribute::System *m_exportSystem;
      smtk::model::GridInfoPtr  m_analysisGridInfo;
      smtk::io::Logger          m_logger;
    };
  }
}


#endif  /* __smtk_simulation_ExportSpec_h */
