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
// .NAME Definition.h - stores the definition of an attribute.
// .SECTION Description
// Stores all of the necessary information for a definition of a
// single attribute. Attributes should be created through
// Manager::createAttribute().
// .SECTION See Also

#ifndef __smtk_attribute_Definition_h
#define __smtk_attribute_Definition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <map>
#include <string>
#include <set>
#include <vector>

namespace smtk
{
  namespace model
  {
    class Item;
  };

  namespace attribute
  {
    class Attribute;
    class ItemDefinition;
    class Manager;

    class SMTKCORE_EXPORT Definition
    {
    public:
      virtual ~Definition();

      // Description:
      // The type is the identifier that is used to access the
      // attribute definition through the Manager. It should never change.
      const std::string &type() const
      { return this->m_type;}

      smtk::attribute::Manager *manager() const
      {return this->m_manager;}

      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      const std::string &label() const
      { return this->m_label;}

      void setLabel(const std::string &newLabel)
      { this->m_label = newLabel;}

      smtk::attribute::DefinitionPtr baseDefinition() const
      { return this->m_baseDefinition;}

      bool isA(smtk::attribute::ConstDefinitionPtr def) const;

      int version() const
      {return this->m_version;}
      void setVersion(int myVersion)
      {this->m_version = myVersion;}

      bool isAbstract() const
      { return this->m_isAbstract;}

      void setIsAbstract(bool isAbstractValue)
      { this->m_isAbstract = isAbstractValue;}

      // The categories that the attribute is associated with. Typically
      // a category will be a simulation type like heat transfer, fluid flow, etc.
      std::size_t numberOfCategories() const
      {return this->m_categories.size();}

      bool isMemberOf(const std::string &category) const
      { return (this->m_categories.find(category) != this->m_categories.end());}

      bool isMemberOf(const std::vector<std::string> &categories) const;

      const std::set<std::string> & categories() const
      {return this->m_categories;}

      // Description:
      // The attributes advance level. 0 is the simplest.
      int advanceLevel() const
      {return this->m_advanceLevel;}
      void setAdvanceLevel(int level)
      {this->m_advanceLevel = level;}

      bool isUnique() const
      {return this->m_isUnique;}
      // Be careful with setting isUnique to be false
      // in order to be consistant all definitions that this is
      // a descendant of should also have isUnique set to false!!
      // isUnique can be set to true without requiring its parent
      // class to also be true.
      void setIsUnique(bool isUniqueValue)
      {this->m_isUnique = isUniqueValue;}

      // Indicates if the attribute applies to the
      // nodes of the analysis mesh
      bool isNodal() const
      { return this->m_isNodal;}
      void setIsNodal(bool isNodalValue)
      {this->m_isNodal = isNodalValue;}

      //Color Specifications
      // Color in the case the attribute does not exist on the model entity
      // If the color has not been set and the def has no base definition it will
      // return s_notApplicableBaseColor
      const double * notApplicableColor() const;
      void setNotApplicableColor(double r, double g, double b, double alpha);
      void setNotApplicableColor(const double *color)
      {this->setNotApplicableColor(color[0], color[1], color[2], color[3]);}
      // By unsetting the color it is now inherited from the def's base definition
      void unsetNotApplicableColor()
      {this->m_isNotApplicableColorSet = false;}
      bool isNotApplicableColorSet() const
      {return this->m_isNotApplicableColorSet;}

      // Default Color for attributes created from this definition -
      // If the color has not been set and the def has no base definition it will
      // return s_defaultBaseColor
      const double * defaultColor() const;
      void setDefaultColor(double r, double g, double b, double alpha);
      void setDefaultColor(const double *color)
      {this->setDefaultColor(color[0], color[1], color[2], color[3]);}
      // By unsetting the color it is now inherited from the def's base definition
      void unsetDefaultColor()
      {this->m_isDefaultColorSet = false;}
      bool isDefaultColorSet() const
      {return this->m_isDefaultColorSet;}

      // Description:
      // Mask is the ability to specify what type of model entities
      // that the attribute can be associated with.
      smtk::model::MaskType associationMask() const
      {return this->m_associationMask;}
      void setAssociationMask(smtk::model::MaskType mask)
      {this->m_associationMask = mask;}
      bool associatesWithVertex() const;
      bool associatesWithEdge() const;
      bool associatesWithFace() const;
      bool associatesWithRegion() const;
      bool associatesWithModel() const;
      bool associatesWithGroup() const;
      bool canBeAssociated(smtk::model::MaskType type) const
      { return (type == (type & this->m_associationMask));}
      // In this case we need to process BCS and DS specially
      // We look at the model's dimension and based on that return
      // the appropriate associatesWith method
      // Conflicts will contain a list of attributes that prevent an attribute
      // of this type from being associated
      bool canBeAssociated(smtk::model::ItemPtr entity,
                           std::vector<smtk::attribute::Attribute *>*conflicts) const;
      bool conflicts(smtk::attribute::DefinitionPtr definition) const;
      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size();}
      smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const
      {
        return (ith < 0) ? smtk::attribute::ItemDefinitionPtr()
          : (ith >= this->m_itemDefs.size() ?
             smtk::attribute::ItemDefinitionPtr() : this->m_itemDefs[ith]);
      }

      // Description:
      // Item definitions are the definitions of what data is stored
      // in the attribute. For example, an IntItemDefinition would store
      // an integer value.
      bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
      template<typename T>
        typename smtk::internal::shared_ptr_type<T>::SharedPointerType
        addItemDefinition(const std::string &name)
      {
        typedef smtk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType item;

        // First see if there is a item by the same name
        if (this->findItemPosition(name) < 0)
          {
          std::size_t n = this->m_itemDefs.size();
          item = SharedTypes::RawPointerType::New(name);
          this->m_itemDefs.push_back(item);
          this->m_itemDefPositions[name] = n;
          }
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

      // Description:
      // Build an attribute corresponding to this definition. If the
      // attribute already has items, clear them out.
      void buildAttribute(smtk::attribute::Attribute *attribute) const;

    protected:
      friend class smtk::attribute::Manager;
      // AttributeDefinitions can only be created by an attribute manager
      Definition(const std::string &myType, smtk::attribute::DefinitionPtr myBaseDef,
                 smtk::attribute::Manager *myManager);

      void clearManager()
      { this->m_manager = NULL;}

      void setCategories();

      smtk::attribute::Manager *m_manager;
      int m_version;
      bool m_isAbstract;
      smtk::attribute::DefinitionPtr m_baseDefinition;
      std::string m_type;
      std::string m_label;
      bool m_isNodal;
      std::set<std::string> m_categories;
      int m_advanceLevel;
      std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
      std::map<std::string, int> m_itemDefPositions;
//Is Unique indicates if more than one attribute of this type can be assigned to a
// model entity - NOTE This can be inherited meaning that if the definition's Super definition
// has isUnique = true it will also prevent an attribute from this definition being assigned if the
// targeted model entity has an attribute derived from the Super Definition
      int m_isUnique;
      bool m_isRequired;
      bool m_isNotApplicableColorSet;
      bool m_isDefaultColorSet;
      smtk::model::MaskType m_associationMask;

      std::string m_detailedDescription;
      std::string m_briefDescription;
    private:

      // These colors are returned for base definitions w/o set colors
      //needs to be private for shiboken wrapping to work properly
      static double s_notApplicableBaseColor[4];
      static double s_defaultBaseColor[4];

      //needs to be private for shiboken wrapping to work properly
      double m_notApplicableColor[4];
      double m_defaultColor[4];

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
//----------------------------------------------------------------------------
    inline const double * Definition::notApplicableColor() const
    {
      if (this->m_isNotApplicableColorSet)
        {
        return this->m_notApplicableColor;
        }
      else if (this->m_baseDefinition)
        {
        return this->m_baseDefinition->notApplicableColor();
        }
      return s_notApplicableBaseColor;
    }
//----------------------------------------------------------------------------
    inline void Definition::setNotApplicableColor(
      double r, double g, double b, double a)
    {
      this->m_isNotApplicableColorSet = true;
      this->m_notApplicableColor[0]= r;
      this->m_notApplicableColor[1]= g;
      this->m_notApplicableColor[2]= b;
      this->m_notApplicableColor[3]= a;
    }
//----------------------------------------------------------------------------
    inline const double * Definition::defaultColor() const
    {
      if (this->m_isDefaultColorSet)
        {
        return this->m_defaultColor;
        }
      else if (this->m_baseDefinition)
        {
        return this->m_baseDefinition->defaultColor();
        }
      return s_defaultBaseColor;
    }
//----------------------------------------------------------------------------
    inline void Definition::setDefaultColor(
      double r, double g, double b, double a)
    {
      this->m_isDefaultColorSet = true;
      this->m_defaultColor[0]= r;
      this->m_defaultColor[1]= g;
      this->m_defaultColor[2]= b;
      this->m_defaultColor[3]= a;
    }
  }
}

#endif /* __smtk_attribute_Definition_h */
