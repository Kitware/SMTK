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
#include "PublicPointerDefs.h"

#include <map>
#include <set>
#include <string>
#include <vector>
 

namespace slctk
{
  namespace attribute
  {
    class Attribute;
    class Definition;
    class RootSection;
    class SLCTKATTRIBUTE_EXPORT Manager
    {
    public:
      
      Manager();
      virtual ~Manager();
      
      slctk::AttributeDefinitionPtr createDefinition(const std::string &typeName,
                                                     const std::string &baseTypeName = "");
      slctk::AttributeDefinitionPtr createDefinition(const std::string &name, 
                                                     AttributeDefinitionPtr baseDefiniiton);
      slctk::AttributePtr createAttribute(const std::string &name, const std::string &type);
      slctk::AttributePtr createAttribute(const std::string &type);
      slctk::AttributePtr createAttribute(const std::string &name, AttributeDefinitionPtr def);
      bool removeAttribute(slctk::AttributePtr att);
      slctk::AttributePtr findAttribute(const std::string &name) const;
      slctk::AttributePtr findAttribute(unsigned long id) const;
      void findAttributes(const std::string &type, std::vector<slctk::AttributePtr> &result) const;
      void findAttributes(slctk::AttributeDefinitionPtr def, std::vector<AttributePtr> &result) const;
      slctk::AttributeDefinitionPtr findDefinition(const std::string &type) const;
      
      // Return a list of definitions that are not derived from another definition
      void findBaseDefinitions(std::vector<slctk::AttributeDefinitionPtr> &result) const;
      
      void derivedDefinitions(slctk::AttributeDefinitionPtr def,
                              std::vector<slctk::AttributeDefinitionPtr> &result) const;

      void findDefinitionAttributes(const std::string &type,
                                    std::vector<slctk::AttributePtr> &result) const;
      void findDefinitions(long mask, std::vector<slctk::AttributeDefinitionPtr> &result) const;
      bool rename(AttributePtr att, const std::string &newName);
      bool defineAnalysis(const std::string &analysisName,
                          const std::set<std::string> &categories);
      std::size_t numberOfAnalyses() const
      {return this->m_analyses.size();}
      std::set<std::string> analysisCategories(const std::string &analysisType) const;
      const std::map<std::string, std::set<std::string> > &analyses() const
      {return this->m_analyses;}

      // For Reader classes
      slctk::AttributePtr createAttribute(const std::string &name, const std::string &type,
                                          unsigned long id);
     slctk::AttributePtr createAttribute(const std::string &name, AttributeDefinitionPtr def,
                                          unsigned long id);
     unsigned long nextId() const
     {return this->m_nextAttributeId;}

     // Sets the next attribute id counter to be the bigger than the largest used by its attributes
     void recomputeNextAttributeID();
     std::string createUniqueName(const std::string &type) const;

      void updateCategories();
      std::size_t numberOfCategories() const
      {return this->m_categories.size();}
      const std::set<std::string> & categories() const
      {return this->m_categories;}

      slctk::RootSectionPtr rootSection() const
      {return this->m_rootSection;}

    protected:
      void internalFindAttributes(AttributeDefinitionPtr def,
                                  std::vector<AttributePtr> &result) const;
      std::map<std::string, slctk::AttributeDefinitionPtr> m_definitions;
      std::map<std::string, std::set<slctk::AttributePtr> > m_attributeClusters;
      std::map<std::string, slctk::AttributePtr> m_attributes;
      std::map<unsigned long, slctk::AttributePtr> m_attributeIdMap;
      std::map<slctk::AttributeDefinitionPtr,
        std::set<slctk::WeakAttributeDefinitionPtr> > m_derivedDefInfo;
      std::set<std::string> m_categories;
      std::map<std::string, std::set<std::string> > m_analyses;
      unsigned long m_nextAttributeId;
      slctk::RootSectionPtr m_rootSection;
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
    inline slctk::AttributePtr Manager::findAttribute(unsigned long attId) const
    {
      std::map<unsigned long, AttributePtr>::const_iterator it;
      it = this->m_attributeIdMap.find(attId);
      return (it == this->m_attributeIdMap.end()) ? slctk::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline slctk::AttributeDefinitionPtr 
    Manager::findDefinition(const std::string &typeName) const
    {
      std::map<std::string, slctk::AttributeDefinitionPtr>::const_iterator it;
      it = this->m_definitions.find(typeName);
      return (it == this->m_definitions.end()) ? slctk::AttributeDefinitionPtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline void  
    Manager::findDefinitionAttributes(const std::string &typeName,
                                      std::vector<slctk::AttributePtr> &result) const
    {
      result.clear();
      std::map<std::string, std::set<slctk::AttributePtr> >::const_iterator it;
      it = this->m_attributeClusters.find(typeName);
      if (it != this->m_attributeClusters.end())
        {
        result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
//----------------------------------------------------------------------------
    inline void Manager::
    findAttributes(const std::string &type, 
                   std::vector<slctk::AttributePtr> &result) const
    {
      result.clear();
      slctk::AttributeDefinitionPtr def = this->findDefinition(type);
      if (def != NULL)
        {
        this->internalFindAttributes(def, result);
        }
    }
//----------------------------------------------------------------------------
  inline std::set<std::string> Manager::
  analysisCategories(const std::string &analysisType) const
  {
    std::map<std::string, std::set<std::string> >::const_iterator it;
      it = this->m_analyses.find(analysisType);
      if (it != this->m_analyses.end())
        {
        return it->second;
        }
      return std::set<std::string>();
  }
//----------------------------------------------------------------------------
    inline bool Manager::defineAnalysis(const std::string &analysisName,
                                        const std::set<std::string> &categories)
    {
    std::map<std::string, std::set<std::string> >::const_iterator it;
      it = this->m_analyses.find(analysisName);
      if (it != this->m_analyses.end())
        {
        // it already exists
        return false;
        }
      this->m_analyses[analysisName] = categories;
      return true;
    }
  };
};


#endif /* __slctk_attribute_Manager_h */
