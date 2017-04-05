//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Definition.h - stores the definition of an attribute.
// .SECTION Description
// Stores all of the necessary information for a definition of a
// single attribute. Attributes should be created through
// System::createAttribute().
// .SECTION See Also

#ifndef __smtk_attribute_Definition_h
#define __smtk_attribute_Definition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h" // For smtkTypeMacro.
#include "smtk/model/EntityRef.h" //for EntityRef version of canBeAssociated
#include "smtk/model/EntityTypeBits.h" // for BitFlags type

#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
  namespace model
  {
    class Item;
  }

  namespace attribute
  {
    class Attribute;
    class ItemDefinition;
    class System;

    class SMTKCORE_EXPORT Definition : public smtk::enable_shared_from_this<Definition>
    {
    public:
      smtkTypeMacro(Definition);
      virtual ~Definition();

      // Description:
      // The type is the identifier that is used to access the
      // attribute definition through the System. It should never change.
      const std::string &type() const
      { return this->m_type;}

      smtk::attribute::System *system() const
      {return this->m_system;}

      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      // By default it is set to the same value as the type.
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

      // The categories that the attribute applies to. Typically
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

      // Indicates if a model entity can have multiple attributes of this
      // type associated with it
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

      ModelEntityItemDefinitionPtr associationRule() const;
      virtual void setAssociationRule(ModelEntityItemDefinitionPtr);

      smtk::model::BitFlags associationMask() const;
      void setAssociationMask(smtk::model::BitFlags mask);

      bool associatesWithVertex() const;
      bool associatesWithEdge() const;
      bool associatesWithFace() const;
      bool associatesWithVolume() const;
      bool associatesWithModel() const;
      bool associatesWithGroup() const;

      bool canBeAssociated(smtk::model::BitFlags maskType) const;
      bool canBeAssociated(smtk::model::EntityRef entity,
        std::vector<smtk::attribute::Attribute *>*conflicts) const;

      bool conflicts(smtk::attribute::DefinitionPtr definition) const;

      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size() + this->m_baseItemOffset;}

      smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const;

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
          this->m_itemDefPositions[name] = static_cast<int>(n);
          this->updateDerivedDefinitions();
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

      // Description:
      // Sets and returns the root name to be used to construct the name for
      // an attribute. This is used by the attribute system when creating an
      // attribute without specifying a name - by default it is set to be the
      // type name of the definition
      void setRootName(const std::string &val)
      {this->m_rootName = val;}
      std::string rootName() const
      {return this->m_rootName;}

      //This method resets the definition item offset - this is used by the
      // system when a definition is modified
      void resetItemOffset();
      std::size_t itemOffset() const
      {return this->m_baseItemOffset;}

    protected:
      friend class smtk::attribute::System;
      // AttributeDefinitions can only be created by an attribute system
      Definition(const std::string &myType, smtk::attribute::DefinitionPtr myBaseDef,
                 smtk::attribute::System *mySystem);

      void clearSystem()
      { this->m_system = NULL;}

      void setCategories();

      // This method updates derived definitions when this
      // definition's items have been changed
      void updateDerivedDefinitions();

      smtk::attribute::System *m_system;
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
      bool m_isUnique;
      bool m_isRequired;
      bool m_isNotApplicableColorSet;
      bool m_isDefaultColorSet;
      smtk::attribute::ModelEntityItemDefinitionPtr m_associationRule;

      std::string m_detailedDescription;
      std::string m_briefDescription;
      // Used by the find method to calculate an item's position
      std::size_t m_baseItemOffset;
      std::string m_rootName;
    private:

      // These colors are returned for base definitions w/o set colors
      //needs to be private for shiboken wrapping to work properly
      static double s_notApplicableBaseColor[4];
      static double s_defaultBaseColor[4];

      //needs to be private for shiboken wrapping to work properly
      double m_notApplicableColor[4];
      double m_defaultColor[4];

    };

    inline void Definition::resetItemOffset()
    {
      if (this->m_baseDefinition)
        {
        this->m_baseItemOffset = this->m_baseDefinition->numberOfItemDefinitions();
        }
    }

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

    inline void Definition::setNotApplicableColor(
      double r, double g, double b, double a)
    {
      this->m_isNotApplicableColorSet = true;
      this->m_notApplicableColor[0]= r;
      this->m_notApplicableColor[1]= g;
      this->m_notApplicableColor[2]= b;
      this->m_notApplicableColor[3]= a;
    }

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
