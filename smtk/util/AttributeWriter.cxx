/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "smtk/util/AttributeWriter.h"
#include "smtk/util/XmlV1StringWriter.h"
#include "smtk/util/Logger.h"
#include <fstream>

using namespace smtk::util;

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
  XmlV1StringWriter theWriter(manager);
  theWriter.includeDefinitions(this->m_includeDefinitions);
  theWriter.includeInstances(this->m_includeInstances);
  theWriter.includeModelInformation(this->m_includeModelInformation);
  theWriter.includeViews(this->m_includeViews);

  std::string result = theWriter.convertToString(logger);
  if(!logger.hasErrors())
	{
	std::ofstream outfile;
	outfile.open(filename.c_str());
	outfile << result;
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
  XmlV1StringWriter theWriter(manager);
  theWriter.includeDefinitions(this->m_includeDefinitions);
  theWriter.includeInstances(this->m_includeInstances);
  theWriter.includeModelInformation(this->m_includeModelInformation);
  theWriter.includeViews(this->m_includeViews);
  filecontents = theWriter.convertToString(logger, no_declaration);
  return logger.hasErrors();
}

//----------------------------------------------------------------------------
