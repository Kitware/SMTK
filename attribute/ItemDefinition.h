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
// .NAME ItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ItemDefinition_h
#define __slctk_attribute_ItemDefinition_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"

#include <string>
#include <set>
#include <vector>

namespace slctk
{
  namespace attribute
  {
    class GroupItemDefinition;
    class Item;
    class Definition;
    class SLCTKATTRIBUTE_EXPORT ItemDefinition
    {
      friend class slctk::attribute::Definition;
      friend class slctk::attribute::GroupItemDefinition;
    public:
      ItemDefinition(const std::string &myname);
      virtual ~ItemDefinition();
      const std::string &name() const
      { return this->m_name;}

      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      const std::string &label() const
      { return this->m_label;}

      void setLabel(const std::string &newLabel)
      { this->m_label = newLabel;}

      int version() const
      {return this->m_version;}
      void setVersion(int myVersion)
      {this->m_version = myVersion;}

      bool isOptional() const
      { return this->m_isOptional;}

      void setIsOptional(bool isOptionalValue)
      { this->m_isOptional = isOptionalValue;}

      std::size_t numberOfCatagories() const
      {return this->m_catagories.size();}

      const std::set<std::string> & catagories() const
      {return this->m_catagories;}

      bool isMemberOf(const std::string &catagory) const
      { return (this->m_catagories.find(catagory) != this->m_catagories.end());}

      bool isMemberOf(const std::vector<std::string> &catagories) const;

      virtual void addCatagory(const std::string &catagory);

      virtual void removeCatagory(const std::string &catagory);

      bool advanceLevel() const
      {return this->m_advanceLevel;}
      void setAdvanceLevel(int level)
      {this->m_advanceLevel = level;}

      const std::string &detailedDescription() const
      {return this->m_detailedDescription;}
      void setDetailedDescription(const std::string &text)
        {this->m_detailedDescription = text;}

      const std::string &briefDescription() const
      {return this->m_briefDescription;}
      void setBriefDescription(const std::string &text)
        {this->m_briefDescription = text;}

      virtual slctk::AttributeItemPtr buildItem() const = 0;
    protected:
      virtual void updateCatagories();
      int m_version;
      bool m_isOptional;
      std::string m_name;
      std::string m_label;
      std::set<std::string> m_catagories;
      int m_advanceLevel;
      std::string m_detailedDescription;
      std::string m_briefDescription;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_ItemDefinition_h */
