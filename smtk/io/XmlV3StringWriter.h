//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlV3StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlV3StringWriter_h
#define __smtk_io_XmlV3StringWriter_h
#include "smtk/io/XmlV2StringWriter.h"
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/System.h"

namespace pugi {
  class xml_node;
}

namespace smtk
{
  namespace io
  {
    class SMTKCORE_EXPORT XmlV3StringWriter : public XmlV2StringWriter
    {
    public:
      XmlV3StringWriter(const smtk::attribute::System &system);
      virtual ~XmlV3StringWriter();

    protected:
      // Override methods
      // Two virtual methods for writing contents
      virtual std::string className() const;
      virtual unsigned int fileVersion() const;

      virtual void processItemDefinitionType(
        pugi::xml_node &node, smtk::attribute::ItemDefinitionPtr idef);
      virtual void processItemType(
        pugi::xml_node &node, smtk::attribute::ItemPtr item);

      // New methods
      void processDateTimeDef(
        pugi::xml_node &node, smtk::attribute::DateTimeItemDefinitionPtr idef);
      void processDateTimeItem(
        pugi::xml_node &node, smtk::attribute::DateTimeItemPtr item);

    private:

    };
  }
}


#endif // __smtk_io_XmlV3StringWriter_h
