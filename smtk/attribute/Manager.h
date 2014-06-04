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

#include "smtk/util/Resource.h"    // base class
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
    class SMTKCORE_EXPORT Manager : public smtk::util::Resource
    {
    public:

      Manager();
      virtual ~Manager();

      virtual smtk::util::Resource::Type resourceType() const;

      smtk::attribute::DefinitionPtr createDefinition(const std::string &typeName,
                                                    const std::string &baseTypeName = "");
      smtk::attribute::DefinitionPtr createDefinition(const std::string &name,
                                                    attribute::DefinitionPtr baseDefiniiton);
      smtk::attribute::AttributePtr createAttribute(const std::string &name, const std::string &type);
      smtk::attribute::AttributePtr createAttribute(const std::string &type);
      smtk::attribute::AttributePtr createAttribute(const std::string &name, attribute::DefinitionPtr def);
      bool removeAttribute(smtk::attribute::AttributePtr att);
      smtk::attribute::AttributePtr findAttribute(const std::string &name) const;
      smtk::attribute::AttributePtr findAttribute(unsigned long id) const;
      void findAttributes(const std::string &type, std::vector<smtk::attribute::AttributePtr> &result) const;
      void findAttributes(smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr> &result) const;
      smtk::attribute::DefinitionPtr findDefinition(const std::string &type) const;

      // Return a list of definitions that are not derived from another definition
      void findBaseDefinitions(std::vector<smtk::attribute::DefinitionPtr> &result) const;

      void derivedDefinitions(smtk::attribute::DefinitionPtr def,
                              std::vector<smtk::attribute::DefinitionPtr> &result) const;

      void findAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool concreteOnly,
                                     std::vector<smtk::attribute::DefinitionPtr> &result) const;

      void findDefinitionAttributes(const std::string &type,
                                    std::vector<smtk::attribute::AttributePtr> &result) const;
      void findDefinitions(unsigned long mask, std::vector<smtk::attribute::DefinitionPtr> &result) const;

      smtk::attribute::ConstDefinitionPtr findIsUniqueBaseClass(
        smtk::attribute::DefinitionPtr attDef) const;

      bool rename(AttributePtr att, const std::string &newName);
      bool defineAnalysis(const std::string &analysisName,
                          const std::set<std::string> &categories);
      std::size_t numberOfAnalyses() const
      {return this->m_analyses.size();}
      std::set<std::string> analysisCategories(const std::string &analysisType) const;
      const std::map<std::string, std::set<std::string> > &analyses() const
      {return this->m_analyses;}

      void addAdvanceLevel(int level, std::string label, const double *l_color=0);
      const std::map<int, std::string> &advanceLevels() const
      {return this->m_advLevels;}
      std::size_t numberOfAdvanceLevels() const
      {return this->m_advLevels.size();}
      // the color is expected in the format of double[4] - rgba
      const double* advanceLevelColor(int level) const;
      void setAdvanceLevelColor(int level, const double *l_color);

      // For Reader classes
      smtk::attribute::AttributePtr createAttribute(const std::string &name, const std::string &type,
                                          unsigned long id);
     smtk::attribute::AttributePtr createAttribute(const std::string &name, attribute::DefinitionPtr def,
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

      smtk::model::ModelPtr refModel() const
        {return this->m_refModel.lock();}
      void setRefModel(smtk::model::ModelPtr refmodel )
        {this->m_refModel = refmodel;}

      smtk::model::ManagerPtr refModelManager() const
        {return this->m_refModelMgr.lock();}
      void setRefModelManager(smtk::model::ManagerPtr refModelMgr);

      bool hasAttributes()
        {return this->m_attributes.size()>0; }

      // When a definition's items has changed use this method to update derived def
      // item offsets which is used by the find item method
      void updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def);

      // Copies definition from another manager
      smtk::attribute::DefinitionPtr copyDefinition(const smtk::attribute::DefinitionPtr def);
    protected:
      void internalFindAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool onlyConcrete,
                                             std::vector<smtk::attribute::DefinitionPtr> &result) const;
      void internalFindAttributes(attribute::DefinitionPtr def,
                                  std::vector<smtk::attribute::AttributePtr> &result) const;
      bool copyDefinitionImpl(const smtk::attribute::DefinitionPtr sourceDef);

      std::map<std::string, smtk::attribute::DefinitionPtr> m_definitions;
      std::map<std::string, std::set<smtk::attribute::AttributePtr> > m_attributeClusters;
      std::map<std::string, smtk::attribute::AttributePtr> m_attributes;
      std::map<unsigned long, smtk::attribute::AttributePtr> m_attributeIdMap;
      std::map<smtk::attribute::DefinitionPtr,
        smtk::attribute::WeakDefinitionPtrSet > m_derivedDefInfo;
      std::set<std::string> m_categories;
      std::map<std::string, std::set<std::string> > m_analyses;
      unsigned long m_nextAttributeId;
      smtk::view::RootPtr m_rootView;

      smtk::model::WeakModelPtr m_refModel;
      smtk::model::WeakManagerPtr m_refModelMgr;
      // Advance levels, <int-level, <string-label, color[4]>
      // higher level means more advanced.
      std::map<int, std::string> m_advLevels;
      std::map<int, std::vector<double> > m_advLevelColors;

    private:
    };
//----------------------------------------------------------------------------
    inline smtk::attribute::AttributePtr Manager::findAttribute(const std::string &name) const
    {
      std::map<std::string, AttributePtr>::const_iterator it;
      it = this->m_attributes.find(name);
      return (it == this->m_attributes.end()) ? smtk::attribute::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline smtk::attribute::AttributePtr Manager::findAttribute(unsigned long attId) const
    {
      std::map<unsigned long, AttributePtr>::const_iterator it;
      it = this->m_attributeIdMap.find(attId);
      return (it == this->m_attributeIdMap.end()) ? smtk::attribute::AttributePtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline smtk::attribute::DefinitionPtr
    Manager::findDefinition(const std::string &typeName) const
    {
      std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
      it = this->m_definitions.find(typeName);
      return (it == this->m_definitions.end()) ? smtk::attribute::DefinitionPtr() : it->second;
    }
//----------------------------------------------------------------------------
    inline void
    Manager::findDefinitionAttributes(const std::string &typeName,
                                      std::vector<smtk::attribute::AttributePtr> &result) const
    {
      result.clear();
      std::map<std::string, std::set<smtk::attribute::AttributePtr> >::const_iterator it;
      it = this->m_attributeClusters.find(typeName);
      if (it != this->m_attributeClusters.end())
        {
        result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
//----------------------------------------------------------------------------
    inline void Manager::
    findAttributes(const std::string &type,
                   std::vector<smtk::attribute::AttributePtr> &result) const
    {
      result.clear();
      smtk::attribute::DefinitionPtr def = this->findDefinition(type);
      if (def)
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
