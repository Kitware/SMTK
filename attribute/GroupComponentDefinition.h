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
// .NAME GroupComponentDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_GroupComponentDefinition_h
#define __slctk_attribute_GroupComponentDefinition_h

#include "attribute/ComponentDefinition.h"
#include <map>
#include <string>
#include <vector>

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT GroupComponentDefinition :
      public ComponentDefinition
    {
    public:
      GroupComponentDefinition(const std::string &myname, 
                               unsigned long myId);
      virtual ~GroupComponentDefinition();
      std::size_t numberOfComponentDefinitions() const
      {return this->m_componentDefs.size();}
      slctk::attribute::ComponentDefinition *componentDefinition(int ith) const
      {
        return (ith < 0) ? NULL : (ith >= this->m_componentDefs.size() ? 
                                   NULL : this->m_componentDefs[ith]);
      }
      bool addComponentDefinition(ComponentDefinition *cdef);
      int findComponentPosition(const std::string &name) const;

      int numberOfGroups() const
      {return this->m_numberOfGroups;}
      void setNumberOfGroups(int gsize)
      {this->m_numberOfGroups = gsize;}
      virtual slctk::AttributeComponentPtr buildComponent() const;
      void buildGroup(std::vector<slctk::AttributeComponentPtr> &group) const;
      
    protected:
      std::vector<slctk::attribute::ComponentDefinition *> m_componentDefs;
      std::map<std::string, int> m_componentDefPositions;
      int m_numberOfGroups;
    private:
    };
//----------------------------------------------------------------------------
    inline int GroupComponentDefinition::
    findComponentPosition(const std::string &name) const
    {
      std::map<std::string, int>::const_iterator it;
      it = this->m_componentDefPositions.find(name);
      if (it == this->m_componentDefPositions.end())
        {
        return -1; // named component doesn't exist
        }
      return it->second;
    }
  };
};


#endif /* __slctk_attribute_GroupComponentDefinition_h */
