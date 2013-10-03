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
// .NAME RootSection.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_RootSection_h
#define __smtk_attribute_RootSection_h

#include "smtk/attribute/GroupSection.h"
namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT RootSection : public GroupSection
    {
    public:
      static smtk::RootSectionPtr New(const std::string &myName)
      { return smtk::RootSectionPtr(new RootSection(myName)); }

      RootSection(const std::string &myTitle);
      virtual ~RootSection();

      virtual Section::Type type() const;

      const double *defaultColor() const
      {return this->m_defaultColor;}
      void setDefaultColor(const double *c)
      {this->setDefaultColor(c[0], c[1], c[2], c[3]);}
      void setDefaultColor(double r, double g, double b, double a);

      const double *invalidColor() const
      {return this->m_invalidColor;}
      void setInvalidColor(const double *c)
      {this->setInvalidColor(c[0], c[1], c[2], c[3]);}
      void setInvalidColor(double r, double g, double b, double a);

    private:
      //needs to be private for shiboken wrapping to work properly
      double m_defaultColor[4];
      double m_invalidColor[4];


    };
//----------------------------------------------------------------------------
    inline void RootSection::setDefaultColor(double r, double g, double b,
                                             double a)
    {
      this->m_defaultColor[0] = r;
      this->m_defaultColor[1] = g;
      this->m_defaultColor[2] = b;
      this->m_defaultColor[3] = a;
    }
//----------------------------------------------------------------------------
    inline void RootSection::setInvalidColor(double r, double g, double b,
                                             double a)
    {
      this->m_invalidColor[0] = r;
      this->m_invalidColor[1] = g;
      this->m_invalidColor[2] = b;
      this->m_invalidColor[3] = a;
    }
//----------------------------------------------------------------------------
  }
}


#endif /* __smtk_attribute_RootSection_h */
