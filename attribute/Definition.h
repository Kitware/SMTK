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
// .NAME Definition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_Definition_h
#define __slctk_attribute_Definition_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
#include <map>
#include <string>
#include <set>
#include <vector>

namespace slctk
{
  class ModelEntity;
  namespace attribute
  {
    class Attribute;
    class ItemDefinition;
    class Manager;
 
    class SLCTKATTRIBUTE_EXPORT Definition
    {
    public:
      friend class slctk::attribute::Manager;
      // Definitions can only be created by an attribute manager
      Definition(const std::string &myType, slctk::AttributeDefinitionPtr myBaseDef,
                 slctk::attribute::Manager *myManager);
      virtual ~Definition();

      const std::string &type() const
      { return this->m_type;}

      slctk::attribute::Manager *manager() const
      {return this->m_manager;}

      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      const std::string &label() const
      { return this->m_label;}

      void setLabel(const std::string &newLabel)
      { this->m_label = newLabel;}

      slctk::AttributeDefinitionPtr baseDefinition() const
      { return this->m_baseDefinition;}

      bool isA(slctk::ConstAttributeDefinitionPtr def) const;

      int version() const
      {return this->m_version;}
      void setVersion(int myVersion)
      {this->m_version = myVersion;}

      bool isAbstract() const
      { return this->m_isAbstract;}

      void setIsAbstract(bool isAbstractValue)
      { this->m_isAbstract = isAbstractValue;}

      std::size_t numberOfCategories() const
      {return this->m_categories.size();}

      bool isMemberOf(const std::string &category) const
      { return (this->m_categories.find(category) != this->m_categories.end());}

      bool isMemberOf(const std::vector<std::string> &categories) const;

      const std::set<std::string> & categories() const
      {return this->m_categories;}

      int advanceLevel() const
      {return this->m_advanceLevel;}
      void setAdvanceLevel(int level)
      {this->m_advanceLevel = level;}

      bool isUnique() const
      {return this->m_isUnique;}
      // Becareful with setting isUnique to be false
      // in order to be consistant all definitions that this is
      // a descendant of should also have isUnique set to false!!
      void setIsUnique(bool isUniqueValue)
      {this->m_isUnique = isUniqueValue;}

      // Indicates if the attribute applies to the
      // nodes of the analysis mesh
      bool isNodal() const
      { return this->m_isNodal;}
      void setIsNodal(bool isNodalValue)
      {this->m_isNodal = isNodalValue;}

      unsigned long associationMask() const
      {return this->m_associationMask;}
      void setAssociationMask(unsigned long mask)
      {this->m_associationMask = mask;}
      bool associatesWithVertex() const
      { return ((this->m_associationMask & 0x1) != 0); }
      bool associatesWithEdge() const
      { return ((this->m_associationMask & 0x2) != 0); }
      bool associatesWithFace() const
      { return ((this->m_associationMask & 0x4) != 0); }
      bool associatesWithRegion() const
      { return ((this->m_associationMask & 0x8) != 0); }
      bool associatesWithModel() const
      { return ((this->m_associationMask & 0x10) != 0); }
      // In this case we need to process BCS and DS specially
      // We look at the model's dimension and based on that return 
      // the appropriate associatesWith method
      // Conflicts will contain a list of attributes that prevent an attribute
      // of this type from being associated
      bool canBeAssociated(slctk::ModelEntity *entity,
                           std::vector<slctk::attribute::Attribute *>*conflicts) const;
      bool conflicts(slctk::AttributeDefinitionPtr definition) const;
      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size();}
      slctk::AttributeItemDefinitionPtr itemDefinition(int ith) const
      {
        return (ith < 0) ? slctk::AttributeItemDefinitionPtr()
          : (ith >= this->m_itemDefs.size() ? 
             slctk::AttributeItemDefinitionPtr() : this->m_itemDefs[ith]);
      }

      bool addItemDefinition(slctk::AttributeItemDefinitionPtr cdef);
      template<typename T>
        typename slctk::internal::shared_ptr_type<T>::SharedPointerType
        addItemDefinition(const std::string &name)
      {
        typedef slctk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType 
          item(new typename SharedTypes::RawPointerType(name));
        this->m_itemDefs.push_back(item);
        return item;
      }

      int findItemPosition(const std::string &name) const;

      const std::string &detailedDescription() const
      {return this->m_detailedDescription;}
      void setDetailedDescription(const std::string &text)
        {this->m_detailedDescription = text;}

      const std::string &briefDescription() const
      {return this->m_briefDescription;}
      void setBriefDescription(const std::string &text)
        {this->m_briefDescription = text;}

      void buildAttribute(slctk::attribute::Attribute *attribute) const;
      slctk::ConstAttributeDefinitionPtr findIsUniqueBaseClass() const;

    protected:
      void clearManager()
      { this->m_manager = NULL;}

      void setCategories();

      slctk::attribute::Manager *m_manager;
      int m_version;
      bool m_isAbstract;
      slctk::AttributeDefinitionPtr m_baseDefinition;
      std::set<WeakAttributeDefinitionPtr> m_derivedDefinitions;
      std::string m_type;
      std::string m_label;
      bool m_isNodal;
      std::set<std::string> m_categories;
      int m_advanceLevel;
      std::vector<slctk::AttributeItemDefinitionPtr> m_itemDefs;
      std::map<std::string, int> m_itemDefPositions;
//Is Unique indicates if more than one attribute of this type can be assigned to a 
// model entity - NOTE This can be inherited meaning that if the definition's Super definition
// has isUnique = true it will also prevent an attribute from this definition being assigned if the
// targeted model entity has an attribute derived from the Super Definition
      int m_isUnique;
      bool m_isRequired;
      unsigned long m_associationMask;
      std::string m_detailedDescription;
      std::string m_briefDescription;
    private:
      
    };
//----------------------------------------------------------------------------
    inline int Definition::findItemPosition(const std::string &name) const
    {
      std::map<std::string, int>::const_iterator it;
      it = this->m_itemDefPositions.find(name);
      if (it == this->m_itemDefPositions.end())
        {
        return -1; // named item doesn't exist
        }
      return it->second;
    }
  };
};

#endif /* __slctk_attribute_Definition_h */
