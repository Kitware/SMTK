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
// .NAME Manager.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_Manager_h
#define __slctk_attribute_Manager_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"

#include <map>
#include <string>
#include <vector>
 

namespace slctk
{
  namespace attribute { class Manager; }
  typedef sharedPtr<attribute::Manager> SharedManager;

  namespace attribute
  {
    class Attribute;
    class Cluster;
    class Definition;

    class SLCTKATTRIBUTE_EXPORT Manager
    {
    public:
      
      Manager();
      virtual ~Manager();
      
      AttributeDefinitionPtr createDefinition(const std::string &typeName,
                                              const std::string &baseTypeName = "");
      slctk::AttributePtr createAttribute(const std::string &name, const std::string &type);
      slctk::AttributePtr createAttribute(const std::string &type);
      slctk::AttributePtr createAttribute(const std::string &name, AttributeDefinitionPtr def);
      bool removeAttribute(slctk::AttributePtr att);
      slctk::AttributePtr findAttribute(const std::string &name) const;
      slctk::AttributeClusterPtr findCluster(const std::string &type) const;
      void findClusters(long mask, std::vector<slctk::AttributeClusterPtr> &result) const;
      bool rename(AttributePtr att, const std::string &newName);
      // For Reader classes
      slctk::AttributePtr createAttribute(const std::string &name, const std::string &type,
                                          unsigned long id);
      void setNextId(unsigned long attributeId)
      {this->m_nextAttributeId = attributeId;}
      std::string createUniqueName(const std::string &type) const;

    protected:
      std::map<std::string, slctk::AttributeClusterPtr> m_clusters;
      std::map<std::string, slctk::AttributePtr> m_attributes;
      std::map<unsigned long, slctk::AttributePtr> m_attributeIdMap;
      unsigned long m_nextAttributeId;
    private:
    };
//----------------------------------------------------------------------------
    inline slctk::AttributePtr Manager::findAttribute(const std::string &name) const
    {
      std::map<std::string, AttributePtr>::const_iterator it;
      it = this->m_attributes.find(name);
      return (it == this->m_attributes.end()) ? slctk::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline slctk::AttributeClusterPtr 
    Manager::findCluster(const std::string &typeName) const
    {
      std::map<std::string, slctk::AttributeClusterPtr>::const_iterator it;
      it = this->m_clusters.find(typeName);
      return (it == this->m_clusters.end()) ? slctk::AttributeClusterPtr() : it->second;
    }
//----------------------------------------------------------------------------
  };
};


#endif /* __slctk_attribute_Manager_h */
