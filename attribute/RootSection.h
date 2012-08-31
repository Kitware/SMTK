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

#ifndef __slctk_attribute_RootSection_h
#define __slctk_attribute_RootSection_h

#include "attribute/GroupSection.h"
namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT RootSection : public GroupSection
    {
    public:
      RootSection(const std::string &myTitle);
      virtual ~RootSection();
      
      virtual Section::Type type() const;
      
      void defaultColor(double c[3]) const
      {this->defaultColor(c[0], c[1], c[2]);}
      void defaultColor(double &r, double &g, double &b) const;
      void setDefaultColor(double c[3])
      {this->setDefaultColor(c[0], c[1], c[2]);}
      void setDefaultColor(double r, double g, double b);
      
      void invalidColor(double c[3]) const
      {this->invalidColor(c[0], c[1], c[2]);}
      void invalidColor(double &r, double &g, double &b) const;
      void setInvalidColor(double c[3])
      {this->setInvalidColor(c[0], c[1], c[2]);}
      void setInvalidColor(double r, double g, double b);
      
    protected:
      double m_defaultColor[3];
      double m_invalidColor[3];
    private:
      
    };
//----------------------------------------------------------------------------
    inline void RootSection::defaultColor(double &r, double &g, 
                                          double &b) const
    {
      r = this->m_defaultColor[0];
      g = this->m_defaultColor[1];
      b = this->m_defaultColor[2];
    }
//----------------------------------------------------------------------------
    inline void RootSection::setDefaultColor(double r, double g, double b)
    {
      this->m_defaultColor[0] = r;
      this->m_defaultColor[1] = g;
      this->m_defaultColor[2] = b;
    }
//----------------------------------------------------------------------------
    inline void RootSection::invalidColor(double &r, double &g, 
                                          double &b) const
    {
      r = this->m_invalidColor[0];
      g = this->m_invalidColor[1];
      b = this->m_invalidColor[2];
    }
//----------------------------------------------------------------------------
    inline void RootSection::setInvalidColor(double r, double g, double b)
    {
      this->m_invalidColor[0] = r;
      this->m_invalidColor[1] = g;
      this->m_defaultColor[2] = b;
    }
//----------------------------------------------------------------------------
  };
};


#endif /* __slctk_attribute_RootSection_h */
