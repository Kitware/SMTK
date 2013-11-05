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
// .NAME Root.h - the root view that other views are contained in
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Root_h
#define __smtk_view_Root_h

#include "smtk/view/Group.h"
namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Root : public Group
    {
    public:
      static smtk::view::RootPtr New(const std::string &myName)
      { return smtk::view::RootPtr(new Root(myName)); }

      Root(const std::string &myTitle);
      virtual ~Root();

      virtual Base::Type type() const;

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

      const bool advancedBold() const
      {return this->m_advancedBold;}
      void setAdvancedBold(const bool b)
      {this->m_advancedBold = b;}

      const bool advancedItalic() const
      {return this->m_advancedItalic;}
      void setAdvancedItalic(const bool i)
      {this->m_advancedItalic = i;}

    private:
      //needs to be private for shiboken wrapping to work properly
      double m_defaultColor[4];
      double m_invalidColor[4];
      // advanced options are bold and not italic by default
      bool m_advancedBold;
      bool m_advancedItalic;
    };
//----------------------------------------------------------------------------
    inline void Root::setDefaultColor(double r, double g, double b,
                                             double a)
    {
      this->m_defaultColor[0] = r;
      this->m_defaultColor[1] = g;
      this->m_defaultColor[2] = b;
      this->m_defaultColor[3] = a;
    }
//----------------------------------------------------------------------------
    inline void Root::setInvalidColor(double r, double g, double b,
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


#endif /* __smtk_view_Root_h */
