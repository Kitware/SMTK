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
// .NAME ColorItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ColorItemDefinition_h
#define __smtk_attribute_ColorItemDefinition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"
#include <cstdio>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class Definition;
    class SMTKCORE_EXPORT ColorItemDefinition:
      public ItemDefinition
    {
    public:
      ColorItemDefinition(const std::string &myName);
      virtual ~ColorItemDefinition();
      
      virtual Item::Type type() const;
      bool isValueValid(double rgb[3]) const;

      virtual smtk::AttributeItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::AttributeItemPtr buildItem(Item *owningItem, 
                                                int position,
                                                int subGroupPosition) const;
      void getDefaultRGB(double& r, double& g, double& b)
        {
        r = this->m_defaultRGB[0];
        g = this->m_defaultRGB[1];
        b = this->m_defaultRGB[2];
        }
      void getDefaultRGB(double rgb[3]) const
        {
        for(int i=0; i<3; i++)
          {
          rgb[i]=m_defaultRGB[i];
          }
        }
      void setDefaultRGB(double rgb[3])
        {
        for(int i=0; i<3; i++)
          {
          m_defaultRGB[i]=rgb[i];
          }
        }
      void setDefaultRGB(double r, double g, double b)
        {
        double rgb[3] = {r, g, b};
        return this->setDefaultRGB(rgb);
        }

      std::string defaultRGBAsString() const
        {
        char dummy[100];
        sprintf(dummy, "%1.2f %1.2f %1.2f", m_defaultRGB[0], m_defaultRGB[1], m_defaultRGB[2]);
        return dummy;
        }

      static bool convertRGBFromString(std::string& dval, double rgb[3]);

    protected:
        double m_defaultRGB[3];
      
    };
  };
};

#endif /* __smtk_attribute_ColorItemDefinition_h */
