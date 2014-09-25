//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

      bool advancedBold() const
      {return this->m_advancedBold;}
      void setAdvancedBold(bool b)
      {this->m_advancedBold = b;}

      bool advancedItalic() const
      {return this->m_advancedItalic;}
      void setAdvancedItalic(bool i)
      {this->m_advancedItalic = i;}

      int maxValueLabelLength() const
      {return this->m_maxValueLabelLen;}
      void setMaxValueLabelLength(int l);
      int minValueLabelLength() const
      {return this->m_minValueLabelLen;}
      void setMinValueLabelLength(int l);

    private:
      //needs to be private for shiboken wrapping to work properly
      double m_defaultColor[4];
      double m_invalidColor[4];
      // advanced options are bold and not italic by default
      bool m_advancedBold;
      bool m_advancedItalic;
      // maximum length for the label of value item, in pixels
      int m_maxValueLabelLen;
      int m_minValueLabelLen;
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
