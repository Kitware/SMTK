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
#include <map>
#include <string>
#include <vector>
namespace slctk
{
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
      
      Definition *createDefinition(const std::string &typeName,
                                   const std::string &baseTypeName = "");
      Attribute *createAttribute(const std::string &name, const std::string &type);
      bool deleteAttribute(Attribute *att);
      Attribute *findAttribute(const std::string &name) const;
      Cluster *findCluster(const std::string &type) const;
      void findClusters(long mask, std::vector<Cluster *> &result) const;
      bool rename(Attribute *att, const std::string &newName);
      // For Reader classes
      Definition *createDefinition(const std::string &typeName,
                                   const std::string &baseTypeName,
                                   unsigned long id);
      Attribute *createAttribute(const std::string &name, const std::string &type,
                                 unsigned long id);
      void setNextIds(unsigned long attributeId, unsigned long definitionId);

    protected:
      std::map<std::string, Cluster *> m_clusters;
      std::map<std::string, Attribute*> m_attributes;
      std::map<unsigned long, Attribute*> m_attributeIdMap;
      unsigned long m_nextAttributeId;
      unsigned long m_nextDefinitionId;
    private:
    };
//----------------------------------------------------------------------------
    inline Attribute *Manager::findAttribute(const std::string &name) const
    {
      std::map<std::string, Attribute *>::const_iterator it;
      it = this->m_attributes.find(name);
      return (it == this->m_attributes.end()) ? NULL : it->second;
    }
//----------------------------------------------------------------------------
    inline Cluster *Manager::findCluster(const std::string &typeName) const
    {
      std::map<std::string, Cluster *>::const_iterator it;
      it = this->m_clusters.find(typeName);
      return (it == this->m_clusters.end()) ? NULL : it->second;
    }
//----------------------------------------------------------------------------
    inline void Manager::setNextIds(unsigned long attributeId, 
                                    unsigned long definitionId)
      { 
        this->m_nextAttributeId = attributeId;
        this->m_nextDefinitionId = definitionId;
      }
//----------------------------------------------------------------------------
  };
};


#endif /* __slctk_attribute_Manager_h */
