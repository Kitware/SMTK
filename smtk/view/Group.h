//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
