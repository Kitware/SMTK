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
// .NAME GroupSection.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_GroupSection_h
#define __slctk_attribute_GroupSection_h

#include "attribute/Section.h"
#include <vector>
namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT GroupSection : public Section
    {
    public:
      GroupSection(const std::string &myTitle);
      virtual ~GroupSection();
      virtual Section::Type type() const;
      std::size_t numberOfSubsections() const
      {return this->m_subSections.size();}
      slctk::SectionPtr subsection(int ith) const
      {return this->m_subSections[ith];}

      template<typename T>
        typename slctk::internal::shared_ptr_type<T>::SharedPointerType
        addSubsection(const std::string &name)
      {
        typedef slctk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType 
          section(new typename SharedTypes::RawPointerType(name));
        this->m_subSections.push_back(section);
        return section;
      }

    protected:
      std::vector<slctk::SectionPtr> m_subSections;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_GroupSection_h */
