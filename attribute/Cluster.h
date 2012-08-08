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
// .NAME Cluster.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_Cluster_h
#define __slctk_attribute_Cluster_h

#include "AttributeExports.h"
#include <string>
#include <map>

namespace slctk
{
  namespace attribute
  {
    class Attribute;
    class Definition;
    class Manager;
    class SLCTKATTRIBUTE_EXPORT Cluster
    {
    public:
      friend class Manager;
      Cluster(slctk::attribute::Manager *myManager, 
              slctk::attribute::Cluster *myParent);
      virtual ~Cluster();
      const std::string &type() const;
      slctk::attribute::Manager *manager() const
      {return this->m_manager;}
      slctk::attribute::Cluster *parent() const
      {return this->m_parent;}
      slctk::attribute::Definition *definition() const
      {return this->m_definition;}
      std::size_t numberOfAttributes() const
      {return this->m_attributes.size();}
      slctk::attribute::Attribute *find(const std::string &name) const;
      bool rename(slctk::attribute::Attribute *att, const std::string &newName);
    protected:
      Definition *generateDefinition(const std::string &typeName,
                                     unsigned long defId);
      Attribute *generateAttribute(const std::string &name,
                                   unsigned long defId);
      void deleteAttribute(slctk::attribute::Attribute *att);
      slctk::attribute::Manager *m_manager;
      slctk::attribute::Cluster *m_parent;
      slctk::attribute::Definition *m_definition;
      std::map<std::string, slctk::attribute::Attribute *> m_attributes;
    private:
    };
//----------------------------------------------------------------------------
    inline  Attribute *Cluster::find(const std::string &name) const
    {
      std::map<std::string, slctk::attribute::Attribute *>::const_iterator it;
      it = this->m_attributes.find(name);
      if (it == this->m_attributes.end())
        {
        return NULL;
        }
      return it->second;
    }
  };
};


#endif /* __slctk_attribute_Cluster_h */
