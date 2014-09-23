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
#include "smtk/util/UUID.h" // for template associatedModelEntities()

#include <map>
#include <set>
#include <string>
#include <vector>


namespace smtk
{
  class ModelEntity;

  namespace attribute
  {
    class RefItem;
    class Item;
    class Manager;
    typedef unsigned long AttributeId;

    class SMTKCORE_EXPORT Attribute
    {
      friend class smtk::attribute::Definition;
      friend class smtk::attribute::Manager;
      friend class smtk::attribute::RefItem;
    public:
      static smtk::attribute::AttributePtr New(const std::string &myName,
                                    smtk::attribute::DefinitionPtr myDefinition,
                                    smtk::attribute::AttributeId myId)
      { return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition, myId)); }

      virtual ~Attribute();
      // NOTE: To rename an attribute use the manager!
      const std::string &name() const
      { return this->m_name;}

      smtk::attribute::AttributeId id() const
      { return this->m_id;}

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

      smtk::attribute::ItemPtr find(const std::string &name) ;
      smtk::attribute::ConstItemPtr find(const std::string &name) const;
      std::size_t numberOfItems() const
      {return this->m_items.size();}

      smtk::attribute::IntItemPtr findInt(const std::string &name);
      smtk::attribute::ConstIntItemPtr findInt(const std::string &name) const;

      smtk::attribute::DoubleItemPtr findDouble(const std::string &name);
      smtk::attribute::ConstDoubleItemPtr findDouble(const std::string &name) const;

      smtk::attribute::StringItemPtr findString(const std::string &name);
      smtk::attribute::ConstStringItemPtr findString(const std::string &name) const;

      smtk::attribute::FileItemPtr findFile(const std::string &name);
      smtk::attribute::ConstFileItemPtr findFile(const std::string &name) const;

      smtk::attribute::DirectoryItemPtr findDirectory(const std::string &name);
      smtk::attribute::ConstDirectoryItemPtr findDirectory(const std::string &name) const;

      smtk::attribute::GroupItemPtr findGroup(const std::string &name);
      smtk::attribute::ConstGroupItemPtr findGroup(const std::string &name) const;

      smtk::attribute::RefItemPtr findRef(const std::string &name);
      smtk::attribute::ConstRefItemPtr findRef(const std::string &name) const;

      smtk::attribute::ModelEntityItemPtr findModelEntity(const std::string &name);
      smtk::attribute::ConstModelEntityItemPtr findModelEntity(const std::string &name) const;

      void references(std::vector<smtk::attribute::ItemPtr> &list) const;

      bool isEntityAssociated(const smtk::util::UUID& entity) const;
      bool isEntityAssociated(const smtk::model::Cursor& cursor) const;

      smtk::util::UUIDs associatedModelEntityIds() const
      {return this->m_modelEntities;}
      template<typename T> T associatedModelEntities() const;

      bool associateEntity(const smtk::util::UUID& entity);

      void disassociateEntity(const smtk::util::UUID& entity, bool reverse = true);
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

      bool isValid();

      smtk::attribute::Manager *manager() const;
      smtk::model::ManagerPtr modelManager() const;

      void setUserData(const std::string &key, smtk::util::UserDataPtr value)
        {this->m_userData[key] = value;}
      smtk::util::UserDataPtr userData(const std::string &key) const;
      void clearUserData(const std::string &key)
        {this->m_userData.erase(key);}
      void clearAllUserData()
        {this->m_userData.clear();}

      bool isAboutToBeDeleted() const
        {return this->m_aboutToBeDeleted;}

    protected:
      Attribute(const std::string &myName,
                smtk::attribute::DefinitionPtr myDefinition, smtk::attribute::AttributeId myId);

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
      std::string m_name;
      std::vector<smtk::attribute::ItemPtr> m_items;
      smtk::attribute::AttributeId m_id;
      smtk::attribute::DefinitionPtr m_definition;
      smtk::util::UUIDs m_modelEntities;
      std::map<smtk::attribute::RefItem *, std::set<std::size_t> > m_references;
      bool m_appliesToBoundaryNodes;
      bool m_appliesToInteriorNodes;
      bool m_isColorSet;
      std::map<std::string, smtk::util::UserDataPtr > m_userData;
      // We need something to indicate that the attribute is in process of
      // being deleted - this is used skip certain clean up steps that
      // would need to be done otherwise
      bool m_aboutToBeDeleted;
    private:
      //needs to be private for shiboken wrapping to work properly
      double m_color[4];

    };
//----------------------------------------------------------------------------
    inline smtk::util::UserDataPtr Attribute::userData(const std::string &key) const
    {
      std::map<std::string, smtk::util::UserDataPtr >::const_iterator it =
        this->m_userData.find(key);
      return ((it == this->m_userData.end()) ? smtk::util::UserDataPtr() : it->second);
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
    template<typename T> T Attribute::associatedModelEntities() const
    {
      T result;
      smtk::model::ManagerPtr modelMgr = this->modelManager();
      if (!modelMgr) { // No attached manager means we cannot make cursors.
        return result;
      }
      smtk::util::UUIDs::const_iterator it;
      for (it = this->m_modelEntities.begin(); it != this->m_modelEntities.end(); ++it) {
        typename T::value_type entry(modelMgr, *it);
        if (entry.isValid()) {
          result.insert(result.end(), entry);
        }
      }
    }
  } // attribute namespace
} // smtk namespace


#endif /* __smtk_attribute_Attribute_h */
