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
// .NAME AttributeWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_AttributeWriter_h
#define __smtk_io_AttributeWriter_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/SystemConfig.h"
#include <string>

namespace smtk
{
  namespace attribute
  {
    class Manager;
  };

  namespace io
  {
    class Logger;
    class SMTKCORE_EXPORT AttributeWriter
    {
    public:
      AttributeWriter();
      // Returns true if there was a problem with writing the file
      bool write(const smtk::attribute::Manager &manager,
                 const std::string &filename,
                 smtk::io::Logger &logger);
      bool writeContents(const smtk::attribute::Manager &manager,
                         std::string &filecontents,
                         smtk::io::Logger &logger,
                         bool no_declaration = false);
      //Control which sections of the attribute manager should be writtern out
      // By Default all sections are processed.  These are advance options!!
      // If val is false then defintions will not be saved
      void includeDefinitions(bool val)
      {this->m_includeDefinitions = val;}

      // If val is false then instances will not be saved
      void includeInstances(bool val)
      {this->m_includeInstances = val;}

      // If val is false then model information will not be saved
      void includeModelInformation(bool val)
      {this->m_includeModelInformation = val;}

      // If val is false then views will not be saved
      void includeViews(bool val)
      {this->m_includeViews = val;}

    protected:
    private:
      bool m_includeDefinitions;
      bool m_includeInstances;
      bool m_includeModelInformation;
      bool m_includeViews;
    };
  }
}


#endif /* __smtk_io_AttributeWriter_h */
