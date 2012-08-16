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
    class Cluster;
    class ComponentDefinition;
    class Manager;
 
    class SLCTKATTRIBUTE_EXPORT Definition
    {
    public:
      // Definitions can only be created by an attribute manager
      Definition(const std::string &myType, slctk::AttributeClusterPtr myCluster);
      virtual ~Definition();

      const std::string &type() const
      { return this->m_type;}

      slctk::attribute::Manager *manager() const;

      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      const std::string &label() const
      { return this->m_label;}

      void setLabel(const std::string &newLabel)
      { this->m_label = newLabel;}

      slctk::AttributeDefinitionPtr baseDefinition() const;
      bool isA(slctk::ConstAttributeDefinitionPtr def) const;

      int version() const
      {return this->m_version;}
      void setVersion(int myVersion)
      {this->m_version = myVersion;}

      bool isAbstract() const
      { return this->m_isAbstract;}

      void setIsAbstract(bool isAbstractValue)
      { this->m_isAbstract = isAbstractValue;}

      std::size_t numberOfCatagories() const
      {return this->m_catagories.size();}

      bool isMemberOf(const std::string &catagory) const
      { return (this->m_catagories.find(catagory) != this->m_catagories.end());}

      bool isMemberOf(const std::vector<std::string> &catagories) const;

      void addCatagory(const std::string &catagory)
      {this->m_catagories.insert(catagory);}

      void removeCatagory(const std::string &catagory)
      {this->m_catagories.erase(catagory);}

      bool advanceLevel() const
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
      bool associatesWithModel() const
      { return ((this->m_associationMask & 0x8) != 0); }
      // In this case we need to process BCS and DS specially
      // We look at the model's dimension and based on that return 
      // the appropriate associatesWith method
      // Conflicts will contain a list of attributes that prevent an attribute
      // of this type from being associated
      bool canBeAssociated(slctk::ModelEntity *entity,
                           std::vector<slctk::attribute::Attribute *>*conflicts) const;
      bool conflicts(slctk::AttributeDefinitionPtr definition) const;
      std::size_t numberOfComponentDefinitions() const
      {return this->m_componentDefs.size();}
      slctk::AttributeComponentDefinitionPtr componentDefinition(int ith) const
      {
        return (ith < 0) ? slctk::AttributeComponentDefinitionPtr()
          : (ith >= this->m_componentDefs.size() ? 
             slctk::AttributeComponentDefinitionPtr() : this->m_componentDefs[ith]);
      }

      bool addComponentDefinition(slctk::AttributeComponentDefinitionPtr cdef);
      template<typename T>
        typename slctk::shared_ptr_type<T>::type addDef(const std::string &name)
      {
        typedef slctk::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::type comp(new typename SharedTypes::T_Type(name));
        this->m_componentDefs.push_back(comp);
        return comp;
      }

      int findComponentPosition(const std::string &name) const;

      const char *detailedDescription() const
      {return this->m_detailedDescription.c_str();}
      void setDetailedDescription(const std::string &text)
        {this->m_detailedDescription = text;}

      const char *briefDescription() const
      {return this->m_briefDescription.c_str();}
      void setBriefDescription(const std::string &text)
        {this->m_briefDescription = text;}

      void buildAttribute(slctk::AttributePtr attribute) const;
      slctk::ConstAttributeDefinitionPtr findIsUniqueBaseClass() const;
      slctk::AttributeClusterPtr cluster() const
      {return this->m_cluster.lock();}

    protected:

      int m_version;
      bool m_isAbstract;
      slctk::WeakAttributeClusterPtr m_cluster;
      std::string m_type;
      std::string m_label;
      bool m_isNodal;
      std::set<std::string> m_catagories;
      int m_advanceLevel;
      std::vector<slctk::AttributeComponentDefinitionPtr> m_componentDefs;
      std::map<std::string, int> m_componentDefPositions;
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
    inline int Definition::findComponentPosition(const std::string &name) const
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

#endif /* __slctk_attribute_Definition_h */
