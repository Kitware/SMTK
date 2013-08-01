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
// .NAME ColorItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ColorItem_h
#define __smtk_attribute_ColorItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class ColorItemDefinition;
    class SMTKCORE_EXPORT ColorItem : public Item
    {
    public:
      ColorItem(Attribute *owningAttribute, int itemPosition);
      ColorItem(Item *owningItem, int position, int subGroupPosition);
      virtual ~ColorItem();
      virtual bool setDefinition(smtk::ConstAttributeItemDefinitionPtr vdef);
      virtual Item::Type type() const;
      const std::string& label() const;
      void getRGB(double color[3]);
      void getRGB(double& r, double& g, double& b);
      bool setRGB(double r, double g, double b);
      bool setRGB(double color[3]);
      virtual void reset();
      virtual bool isSet() const
      {return this->m_isSet;}
    protected:
      double m_rgb[3];
      bool m_isSet;
    private:
    };
  };
};

#endif /* __smtk_attribute_ColorItem_h */
