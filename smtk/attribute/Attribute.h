//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Attribute_h
#define __smtk_attribute_Attribute_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/SearchStyle.h"

#include "smtk/common/UUID.h" // for template associatedModelEntities()

#include <map>
#include <set>
#include <string>
#include <vector>


namespace smtk
{
  class Model;

  namespace attribute
  {
    class RefItem;
    class Item;
    class System;

    /**\brief Represent a (possibly composite) value according to a definition.
      *
      */
    class SMTKCORE_EXPORT Attribute
    {
      friend class smtk::attribute::Definition;
      friend class smtk::attribute::System;
      friend class smtk::attribute::RefItem;
    public:
      static smtk::attribute::AttributePtr
        New(const std::string &myName,
            smtk::attribute::DefinitionPtr myDefinition)
      { return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition)); }

       static smtk::attribute::AttributePtr
        New(const std::string &myName,
            smtk::attribute::DefinitionPtr myDefinition,
            const smtk::common::UUID &myId)
      { return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition, myId)); }

      virtual ~Attribute();

      // NOTE: To rename an attribute use the System!
      const std::string& name() const { return this->m_name;}
      const smtk::common::UUID& id() const { return this->m_id;}

      const std::string &type() const;
      std::vector<std::string> types() const;
      bool isA(smtk::attribute::DefinitionPtr def) const;
      smtk::attribute::DefinitionPtr definition() const
      {return this->m_definition;}

      const double *color() const;
      void setColor(double r, double g, double b, double alpha);
      void setColor(const double *l_color)
      {this->setColor(l_color[0], l_color[1], l_color[2], l_color[3]);}
      bool isColorSet() const
      {return this->m_isColorSet;}
      void unsetColor()
      {this->m_isColorSet = false;}

      // Return the public pointer for this attribute.
      smtk::attribute::AttributePtr pointer() const;

      bool isMemberOf(const std::string &category) const;
      bool isMemberOf(const std::vector<std::string> &categories) const;

      smtk::attribute::ItemPtr item(int ith) const
      {
        return (ith < 0) ? smtk::attribute::ItemPtr() :
          (static_cast<unsigned int>(ith) >= this->m_items.size() ?
           smtk::attribute::ItemPtr() : this->m_items[static_cast<std::size_t>(ith)]);
      }

      smtk::attribute::ItemPtr find(
        const std::string& name,
        SearchStyle style = ACTIVE_CHILDREN);
      smtk::attribute::ConstItemPtr find(
        const std::string &name,
        SearchStyle style = ACTIVE_CHILDREN) const;
      std::size_t numberOfItems() const
      {return this->m_items.size();}

      template<typename T>
      typename T::Ptr findAs(
        const std::string& name,
        SearchStyle style = ACTIVE_CHILDREN);

      IntItemPtr findInt(const std::string &name);
      ConstIntItemPtr findInt(const std::string &name) const;

      DoubleItemPtr findDouble(const std::string &name);
      ConstDoubleItemPtr findDouble(const std::string &name) const;

      StringItemPtr findString(const std::string &name);
      ConstStringItemPtr findString(const std::string &name) const;

      FileItemPtr findFile(const std::string &name);
      ConstFileItemPtr findFile(const std::string &name) const;

      DirectoryItemPtr findDirectory(const std::string &name);
      ConstDirectoryItemPtr findDirectory(const std::string &name) const;

      GroupItemPtr findGroup(const std::string &name);
      ConstGroupItemPtr findGroup(const std::string &name) const;

      RefItemPtr findRef(const std::string &name);
      ConstRefItemPtr findRef(const std::string &name) const;

      ModelEntityItemPtr findModelEntity(const std::string &name);
      ConstModelEntityItemPtr findModelEntity(const std::string &name) const;
      template<typename T> T modelEntitiesAs(const std::string& name) const;

      void references(std::vector<smtk::attribute::ItemPtr> &list) const;

      ConstModelEntityItemPtr associations() const { return this->m_associations; }
      ModelEntityItemPtr associations() { return this->m_associations; }

      bool isEntityAssociated(const smtk::common::UUID& entity) const;
      bool isEntityAssociated(const smtk::model::EntityRef& entityref) const;

      smtk::common::UUIDs associatedModelEntityIds() const;
      template<typename T> T associatedModelEntities() const;

      bool associateEntity(const smtk::common::UUID& entity);
      bool associateEntity(const smtk::model::EntityRef& entity);

      void disassociateEntity(const smtk::common::UUID& entity, bool reverse = true);
      void disassociateEntity(const smtk::model::EntityRef& entity, bool reverse = true);
      void removeAllAssociations();

      MeshEntityItemPtr findMeshEntity(const std::string &name);
      ConstMeshEntityItemPtr findMeshEntity(const std::string &name) const;

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

      bool isValid();

      smtk::attribute::System* system() const;
      smtk::model::ManagerPtr modelManager() const;

      void setUserData(const std::string &key, smtk::simulation::UserDataPtr value)
        {this->m_userData[key] = value;}
      smtk::simulation::UserDataPtr userData(const std::string &key) const;
      void clearUserData(const std::string &key)
        {this->m_userData.erase(key);}
      void clearAllUserData()
        {this->m_userData.clear();}

      bool isAboutToBeDeleted() const
        {return this->m_aboutToBeDeleted;}

    protected:
      Attribute(const std::string &myName,
                smtk::attribute::DefinitionPtr myDefinition, const smtk::common::UUID &myId);
      Attribute(const std::string &myName,
                smtk::attribute::DefinitionPtr myDefinition);

      void removeAllItems();
      void addItem(smtk::attribute::ItemPtr iPtr)
      {this->m_items.push_back(iPtr);}
      void setName(const std::string &newname)
      {this->m_name = newname;}

      void addReference(smtk::attribute::RefItem *attRefItem, std::size_t pos)
        {this->m_references[attRefItem].insert(pos);}
      // This removes a specific ref item
      void removeReference(smtk::attribute::RefItem *attRefItem, std::size_t pos)
        {this->m_references[attRefItem].erase(pos);}
      // This removes all references to a specific Ref Item
      void removeReference(smtk::attribute::RefItem *attRefItem)
        {this->m_references.erase(attRefItem);}

#ifndef SHIBOKEN_SKIP
      std::string m_name;
      std::vector<smtk::attribute::ItemPtr> m_items;
      ModelEntityItemPtr m_associations;
      smtk::common::UUID m_id;
      smtk::attribute::DefinitionPtr m_definition;
      std::map<smtk::attribute::RefItem *, std::set<std::size_t> > m_references;
      bool m_appliesToBoundaryNodes;
      bool m_appliesToInteriorNodes;
      bool m_isColorSet;
      std::map<std::string, smtk::simulation::UserDataPtr > m_userData;
      // We need something to indicate that the attribute is in process of
      // being deleted - this is used skip certain clean up steps that
      // would need to be done otherwise
      bool m_aboutToBeDeleted;
      double m_color[4];
#endif // SHIBOKEN_SKIP
    };
//----------------------------------------------------------------------------
    inline smtk::simulation::UserDataPtr Attribute::userData(const std::string &key) const
    {
      std::map<std::string, smtk::simulation::UserDataPtr >::const_iterator it =
        this->m_userData.find(key);
      return ((it == this->m_userData.end()) ? smtk::simulation::UserDataPtr() : it->second);
    }
//----------------------------------------------------------------------------
    inline void Attribute::setColor(double r, double g, double b, double a)
    {
      this->m_isColorSet = true;
      this->m_color[0]= r;
      this->m_color[1]= g;
      this->m_color[2]= b;
      this->m_color[3]= a;
    }

//----------------------------------------------------------------------------
    template<typename T> T Attribute::modelEntitiesAs(const std::string& iname) const
    {
      T result;
      ConstModelEntityItemPtr itm = this->findModelEntity(iname);
      if (!itm)
        return result;

      smtk::model::EntityRefArray::const_iterator it;
      for (it = itm->begin(); it != itm->end(); ++it) {
        typename T::value_type entry(*it);
        if (entry.isValid()) {
          result.insert(result.end(), entry);
        }
      }
      return result;
    }
//----------------------------------------------------------------------------
    template<typename T> T Attribute::associatedModelEntities() const
    {
      T result;
      if (!this->m_associations)
        return result;

      smtk::model::EntityRefArray::const_iterator it;
      for (it = this->m_associations->begin(); it != this->m_associations->end(); ++it) {
        typename T::value_type entry(*it);
        if (entry.isValid()) {
          result.insert(result.end(), entry);
        }
      }
      return result;
    }
//----------------------------------------------------------------------------
    template<typename T>
    typename T::Ptr Attribute::findAs(const std::string& iname, SearchStyle style)
    {
    return smtk::dynamic_pointer_cast<T>(this->find(iname, style));
    }
//----------------------------------------------------------------------------
  } // attribute namespace
} // smtk namespace


#endif /* __smtk_attribute_Attribute_h */
