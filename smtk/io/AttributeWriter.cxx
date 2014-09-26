//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/AttributeWriter.h"
#include "smtk/io/XmlV2StringWriter.h"
#include "smtk/io/Logger.h"
#include <fstream>

namespace smtk {
  namespace io {

//----------------------------------------------------------------------------
AttributeWriter::AttributeWriter():
  m_includeDefinitions(true), m_includeInstances(true),
  m_includeModelInformation(true), m_includeViews(true)
{
}
//----------------------------------------------------------------------------
bool AttributeWriter::write(const smtk::attribute::Manager &manager,
                            const std::string &filename,
                            Logger &logger)
{
  logger.reset();
  XmlV2StringWriter theWriter(manager);
  theWriter.includeDefinitions(this->m_includeDefinitions);
  theWriter.includeInstances(this->m_includeInstances);
  theWriter.includeModelInformation(this->m_includeModelInformation);
  theWriter.includeViews(this->m_includeViews);

  std::string result = theWriter.convertToString(logger);
  if(!logger.hasErrors())
	{
	std::ofstream outfile;
	outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
        if (!outfile)
          {
          smtkErrorMacro(logger, "Error opening file for writing: " << filename);
          }
        else
          {
          outfile << result;
          }
	outfile.close();
	}
  return logger.hasErrors();
}

//----------------------------------------------------------------------------
bool AttributeWriter::writeContents(const smtk::attribute::Manager &manager,
                                    std::string &filecontents,
                                    Logger &logger,
                                    bool no_declaration)
{
  logger.reset();
  XmlV2StringWriter theWriter(manager);
  theWriter.includeDefinitions(this->m_includeDefinitions);
  theWriter.includeInstances(this->m_includeInstances);
  theWriter.includeModelInformation(this->m_includeModelInformation);
  theWriter.includeViews(this->m_includeViews);
  filecontents = theWriter.convertToString(logger, no_declaration);
  return logger.hasErrors();
}

//----------------------------------------------------------------------------

  } // namespace io
} // namespace smtk
