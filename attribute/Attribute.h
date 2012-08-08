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
// .NAME slctkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_Attribute_h
#define __slctk_attribute_Attribute_h

#include "AttributeExports.h"

#include <tr1/memory>
#include <set>
#include <string>
#include <vector>


namespace slctk
{
  class ModelEntity;

  namespace attribute
  {
    class AttributeReferenceComponent;
    class Component;
    class Definition;
    class Cluster;
    class Manager;

    class SLCTKATTRIBUTE_EXPORT Attribute
    {
      friend class slctk::attribute::AttributeReferenceComponent;
      friend class slctk::attribute::Cluster;
      friend class slctk::attribute::Definition;
    public:
      Attribute(const std::string &myName,
                slctk::attribute::Cluster *myCluster, unsigned long myId);
      virtual ~Attribute();
      // NOTE: To rename an attribute use the manager!
      const std::string &name() const
      { return this->m_name;}

      unsigned long id() const
      { return this->m_id;}

      const std::string &type() const;
      std::vector<std::string> types() const;
      bool isA(slctk::attribute::Definition *def) const;
      const slctk::attribute::Definition *definition() const;

      bool isMemberOf(const std::string &catagory) const;
      bool isMemberOf(const std::vector<std::string> &catagories) const;

      slctk::attribute::Component *component(int ith) const
      {
        return (ith < 0) ? NULL : (ith >= this->m_components.size() ? 
                                   NULL : this->m_components[ith]);
      }

      slctk::attribute::Component *find(const std::string &name) ;
      const slctk::attribute::Component *find(const std::string &name) const;
      std::size_t numberOfComponents() const
      {return this->m_components.size();}

      std::size_t numberOfAssociatedEntities() const
      { return this->m_entities.size();}
      bool isEntityAssociated(slctk::ModelEntity *entity) const
      { return (this->m_entities.find(entity) != this->m_entities.end());}
      std::set<slctk::ModelEntity *>::const_iterator associatedEntities() const
      {return this->m_entities.begin();}
      void associateEntity(slctk::ModelEntity *entity);
      void disassociateEntity(slctk::ModelEntity *entity);
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

      bool isReferenced() const
      {return this->m_references.size() != 0;}

      slctk::attribute::Manager *manager() const;

      slctk::attribute::Cluster *cluster() const
      {return this->m_cluster;}

    protected:
      void removeAllComponents();
      void addComponent(slctk::attribute::Component *component)
      {this->m_components.push_back(component);}
      void setName(const std::string &newname)
      {this->m_name = newname;}

      void registerComponent(slctk::attribute::AttributeReferenceComponent *comp)
      {this->m_references.insert(comp);}
      void unregisterComponent(slctk::attribute::AttributeReferenceComponent *comp)
      {this->m_references.erase(comp);}
     
      struct internal_no_copy_data
      {
      };
      std::string m_name;
      std::vector<slctk::attribute::Component *> m_components;
      unsigned long m_id;
      slctk::attribute::Cluster *m_cluster;
      std::set<slctk::ModelEntity *> m_entities;
      std::set<slctk::attribute::AttributeReferenceComponent *> m_references;
      bool m_appliesToBoundaryNodes;
      bool m_appliesToInteriorNodes;

      std::tr1::shared_ptr<internal_no_copy_data> Data;
    private:
      
    };
  };
};


#endif /* __slctk_attribute_Attribute_h */
