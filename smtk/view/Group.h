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
// .NAME GroupSection.h - a section that can have sub-sections
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_GroupSection_h
#define __smtk_view_GroupSection_h

#include "smtk/view/Base.h"
#include <vector>
namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Group : public Base
    {
    public:
       enum Style
      {
        TABBED,
        TILED
      };

      static smtk::view::GroupPtr New(const std::string &myName)
      { return smtk::view::GroupPtr(new smtk::view::Group(myName)); }

      Group(const std::string &myTitle);
      virtual ~Group();
      virtual Base::Type type() const;
      std::size_t numberOfSubViews() const
      {return this->m_subViews.size();}
      smtk::view::BasePtr subView(std::size_t ith) const
      {return this->m_subViews[ith];}

      bool addSubView( smtk::view::BasePtr subview )
      {
        this->m_subViews.push_back(subview);
        return true;
      }

      virtual Group::Style style() const
      { return this->m_style; }
      virtual void setStyle(Group::Style aStyle)
      { this->m_style = aStyle; }

      template<typename T>
        typename smtk::internal::shared_ptr_type<T>::SharedPointerType
        addSubView(const std::string &name)
      {
        typedef smtk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType subview;
        subview = SharedTypes::RawPointerType::New(name);
        this->m_subViews.push_back(subview);
        return subview;
      }

    protected:
      std::vector<smtk::view::BasePtr> m_subViews;
      Group::Style m_style;

    private:

    };
  }
}

#endif /* __smtk_view_Group_h */
