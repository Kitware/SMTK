//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Attribute.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Attribute_h
#define __smtk_view_Attribute_h

#include "smtk/view/Base.h"
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Attribute : public Base
    {
    public:
      static smtk::view::AttributePtr New(const std::string &myTitle)
      { return smtk::view::AttributePtr(new Attribute(myTitle)); }

      Attribute(const std::string &myTitle);
      virtual ~Attribute();
      virtual Base::Type type() const;
      void addDefinition(smtk::attribute::DefinitionPtr def)
      {this->m_definitions.push_back(def);}
      std::size_t numberOfDefinitions() const
      { return this->m_definitions.size();}
      smtk::attribute::DefinitionPtr definition(std::size_t ith) const
      {return this->m_definitions[ith];}
      smtk::model::BitFlags modelEntityMask() const
      {return this->m_modelEntityMask;}
      void setModelEntityMask(smtk::model::BitFlags mask)
      {this->m_modelEntityMask = mask;}
      bool okToCreateModelEntities() const
      { return this->m_okToCreateModelEntities;}
      void setOkToCreateModelEntities(bool val)
      { this->m_okToCreateModelEntities = val;}

    protected:
      std::vector<smtk::attribute::DefinitionPtr> m_definitions;
      smtk::model::BitFlags m_modelEntityMask;
      bool m_okToCreateModelEntities;

    private:
    };
  }
}


#endif /* __smtk_view_Attribute_h */
