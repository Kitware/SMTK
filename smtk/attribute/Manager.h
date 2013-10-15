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
// .NAME Manager.h - the main class for storing attribute information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Manager_h
#define __smtk_attribute_Manager_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <set>
#include <string>
#include <vector>


namespace smtk
{
  namespace view
  {
    class Root;
  }

  namespace attribute
  {
    class Attribute;
    class Definition;
    class SMTKCORE_EXPORT Manager
    {
    public:
      // static set and get methods for setting a Manager that can be accessed
      // through the shiboken wrapping. we probably want to do something better
      // in the future but I think that may require wrapping parts of CMB with
      // shiboken.
      static Manager* getGlobalManager();
      static void setGlobalManager(Manager* m);

      Manager();
      virtual ~Manager();

      smtk::AttributeDefinitionPtr createAttributeDefinition(const std::string &typeName,
                                                             const std::string &baseTypeName = "");
      smtk::AttributeDefinitionPtr createAttributeDefinition(const std::string &name,
                                                             AttributeDefinitionPtr baseDefiniiton);
      smtk::AttributePtr createAttribute(const std::string &name, const std::string &type);
      smtk::AttributePtr createAttribute(const std::string &type);
      smtk::AttributePtr createAttribute(const std::string &name, AttributeDefinitionPtr def);
      bool removeAttribute(smtk::AttributePtr att);
      smtk::AttributePtr findAttribute(const std::string &name) const;
      smtk::AttributePtr findAttribute(unsigned long id) const;
      void findAttributes(const std::string &type, std::vector<smtk::AttributePtr> &result) const;
      void findAttributes(smtk::AttributeDefinitionPtr def, std::vector<smtk::AttributePtr> &result) const;
      smtk::AttributeDefinitionPtr findDefinition(const std::string &type) const;

      // Return a list of definitions that are not derived from another definition
      void findBaseDefinitions(std::vector<smtk::AttributeDefinitionPtr> &result) const;

      void derivedDefinitions(smtk::AttributeDefinitionPtr def,
                              std::vector<smtk::AttributeDefinitionPtr> &result) const;

      void findAllDerivedDefinitions(smtk::AttributeDefinitionPtr def, bool concreteOnly,
                                     std::vector<smtk::AttributeDefinitionPtr> &result) const;

      void findDefinitionAttributes(const std::string &type,
                                    std::vector<smtk::AttributePtr> &result) const;
      void findDefinitions(long mask, std::vector<smtk::AttributeDefinitionPtr> &result) const;

      smtk::ConstAttributeDefinitionPtr findIsUniqueBaseClass(
        smtk::AttributeDefinitionPtr attDef) const;

      bool rename(AttributePtr att, const std::string &newName);
      bool defineAnalysis(const std::string &analysisName,
                          const std::set<std::string> &categories);
      std::size_t numberOfAnalyses() const
      {return this->m_analyses.size();}
      std::set<std::string> analysisCategories(const std::string &analysisType) const;
      const std::map<std::string, std::set<std::string> > &analyses() const
      {return this->m_analyses;}

      // For Reader classes
      smtk::AttributePtr createAttribute(const std::string &name, const std::string &type,
                                          unsigned long id);
     smtk::AttributePtr createAttribute(const std::string &name, AttributeDefinitionPtr def,
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

      smtk::view::RootPtr rootView() const
      {return this->m_rootView;}

      smtk::ModelPtr refModel() const
        {return this->m_refModel.lock();}
      void setRefModel(smtk::ModelPtr refmodel )
        {this->m_refModel = refmodel;}

      bool hasAttributes()
        {return this->m_attributes.size()>0; }

    protected:
      void internalFindAllDerivedDefinitions(smtk::AttributeDefinitionPtr def, bool onlyConcrete,
                                             std::vector<smtk::AttributeDefinitionPtr> &result) const;
      void internalFindAttributes(AttributeDefinitionPtr def,
                                  std::vector<smtk::AttributePtr> &result) const;
      std::map<std::string, smtk::AttributeDefinitionPtr> m_definitions;
      std::map<std::string, std::set<smtk::AttributePtr> > m_attributeClusters;
      std::map<std::string, smtk::AttributePtr> m_attributes;
      std::map<unsigned long, smtk::AttributePtr> m_attributeIdMap;
      std::map<smtk::AttributeDefinitionPtr,
        smtk::WeakAttributeDefinitionPtrSet > m_derivedDefInfo;
      std::set<std::string> m_categories;
      std::map<std::string, std::set<std::string> > m_analyses;
      unsigned long m_nextAttributeId;
      smtk::view::RootPtr m_rootView;

      smtk::WeakModelPtr m_refModel;
    private:
    };
//----------------------------------------------------------------------------
    inline smtk::AttributePtr Manager::findAttribute(const std::string &name) const
    {
      std::map<std::string, AttributePtr>::const_iterator it;
      it = this->m_attributes.find(name);
      return (it == this->m_attributes.end()) ? smtk::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline smtk::AttributePtr Manager::findAttribute(unsigned long attId) const
    {
      std::map<unsigned long, AttributePtr>::const_iterator it;
      it = this->m_attributeIdMap.find(attId);
      return (it == this->m_attributeIdMap.end()) ? smtk::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline smtk::AttributeDefinitionPtr
    Manager::findDefinition(const std::string &typeName) const
    {
      std::map<std::string, smtk::AttributeDefinitionPtr>::const_iterator it;
      it = this->m_definitions.find(typeName);
      return (it == this->m_definitions.end()) ? smtk::AttributeDefinitionPtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline void
    Manager::findDefinitionAttributes(const std::string &typeName,
                                      std::vector<smtk::AttributePtr> &result) const
    {
      result.clear();
      std::map<std::string, std::set<smtk::AttributePtr> >::const_iterator it;
      it = this->m_attributeClusters.find(typeName);
      if (it != this->m_attributeClusters.end())
        {
        result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
//----------------------------------------------------------------------------
    inline void Manager::
    findAttributes(const std::string &type,
                   std::vector<smtk::AttributePtr> &result) const
    {
      result.clear();
      smtk::AttributeDefinitionPtr def = this->findDefinition(type);
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
                                        const std::set<std::string> &categoriesIn)
    {
    std::map<std::string, std::set<std::string> >::const_iterator it;
      it = this->m_analyses.find(analysisName);
      if (it != this->m_analyses.end())
        {
        // it already exists
        return false;
        }
      this->m_analyses[analysisName] = categoriesIn;
      return true;
    }
  }
}


#endif /* __smtk_attribute_Manager_h */
