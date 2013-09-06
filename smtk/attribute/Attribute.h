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
// .NAME smtkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Attribute_h
#define __smtk_attribute_Attribute_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <set>
#include <string>
#include <vector>


namespace smtk
{
  class ModelEntity;

  namespace attribute
  {
    class AttributeRefItem;
    class Item;
    class Definition;
    class Manager;

    class SMTKCORE_EXPORT Attribute
    {
      friend class smtk::attribute::Definition;
      friend class smtk::attribute::Manager;
      friend class smtk::attribute::AttributeRefItem;
    public:
      Attribute(const std::string &myName,
                smtk::AttributeDefinitionPtr myDefinition, unsigned long myId);
      virtual ~Attribute();
      // NOTE: To rename an attribute use the manager!
      const std::string &name() const
      { return this->m_name;}

      unsigned long id() const
      { return this->m_id;}

      const std::string &type() const;
      std::vector<std::string> types() const;
      bool isA(smtk::AttributeDefinitionPtr def) const;
      smtk::AttributeDefinitionPtr definition() const
      {return this->m_definition;}

      const double *color() const;
      void setColor(double r, double g, double b, double alpha);
      void setColor(const double *color)
      {this->setColor(color[0], color[1], color[2], color[3]);}
      bool isColorSet() const
      {return this->m_isColorSet;}
      void unsetColor()
      {this->m_isColorSet = false;}

      // Return the public pointer for this attribute.
      smtk::AttributePtr pointer() const;

      bool isMemberOf(const std::string &category) const;
      bool isMemberOf(const std::vector<std::string> &categories) const;

      smtk::AttributeItemPtr item(int ith) const
      {
        return (ith < 0) ? smtk::AttributeItemPtr() :
          (ith >= this->m_items.size() ?
           smtk::AttributeItemPtr() : this->m_items[ith]);
      }

      smtk::AttributeItemPtr find(const std::string &name) ;
      smtk::ConstAttributeItemPtr find(const std::string &name) const;
      std::size_t numberOfItems() const
      {return this->m_items.size();}

      void references(std::vector<smtk::AttributeItemPtr> &list) const;

      std::size_t numberOfAssociatedEntities() const
      { return this->m_entities.size();}
      bool isEntityAssociated(smtk::ModelItemPtr entity) const
      { return (this->m_entities.find(entity) != this->m_entities.end());}
      std::set<smtk::ModelItemPtr>::const_iterator associatedEntities() const
      {return this->m_entities.begin();}
      void associateEntity(smtk::ModelItemPtr entity);
      void disassociateEntity(smtk::ModelItemPtr entity, bool reverse=true);
      void removeAllAssociations();

      // These methods only applies to Attributes whose
      // definition returns true for isNodal()
      bool appliesToBoundaryNodes() const
      {return this->m_appliesToBoundaryNodes;}
      void setAppliesToBoundaryNodes(bool appliesValue)
      {this->m_appliesToBoundaryNodes = appliesValue;}
      bool appliesToInteriorNodes() const
      {return this->m_appliesToInteriorNodes;}
      void setAppliesToInteriorNodes(bool appliesValue)
      {this->m_appliesToInteriorNodes = appliesValue;}

      smtk::attribute::Manager *manager() const;
      // void setUserData(const std::string &key, void *value)
      // {this->m_userData[key] = value;}
      // void *userData(const std::string &key) const;
      // void clearUserData(const std::string &key)
      // {this->m_userData.erase(key);}
      // void clearAllUserData()
      // {this->m_userData.clear();}
      bool isAboutToBeDeleted() const
      {return this->m_aboutToBeDeleted;}

    protected:
      void removeAllItems();
      void addItem(smtk::AttributeItemPtr item)
      {this->m_items.push_back(item);}
      void setName(const std::string &newname)
      {this->m_name = newname;}

      void addReference(smtk::attribute::AttributeRefItem *attRefItem, int pos)
        {this->m_references[attRefItem].insert(pos);}
      // This removes a specific ref item
      void removeReference(smtk::attribute::AttributeRefItem *attRefItem, int pos)
        {this->m_references[attRefItem].erase(pos);}
      // This removes all references to a specific Ref Item
      void removeReference(smtk::attribute::AttributeRefItem *attRefItem)
        {this->m_references.erase(attRefItem);}
      std::string m_name;
      std::vector<smtk::AttributeItemPtr> m_items;
      unsigned long m_id;
      smtk::AttributeDefinitionPtr m_definition;
      std::set<smtk::ModelItemPtr> m_entities;
      std::map<smtk::attribute::AttributeRefItem *, std::set<int> > m_references;
      bool m_appliesToBoundaryNodes;
      bool m_appliesToInteriorNodes;
      bool m_isColorSet;
      // std::map<std::string, void *> m_userData;
      // We need something to indicate that the attribute is in process of
      // being deleted - this is used skip certain clean up steps that
      // would need to be done otherwise
      bool m_aboutToBeDeleted;
    private:
      //needs to be private for shiboken wrapping to work properly
      double m_color[4];

    };
//----------------------------------------------------------------------------
    // inline void *Attribute::userData(const std::string &key) const
    // {
    //   std::map<std::string, void *>::const_iterator it =
    //     this->m_userData.find(key);
    //   return ((it == this->m_userData.end()) ? NULL : it->second);
    // }
//----------------------------------------------------------------------------
    inline void Attribute::setColor(double r, double g, double b, double a)
    {
      this->m_isColorSet = true;
      this->m_color[0]= r;
      this->m_color[1]= g;
      this->m_color[2]= b;
      this->m_color[3]= a;
    }
  };
};


#endif /* __smtk_attribute_Attribute_h */
