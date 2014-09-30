//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ModelEntity.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_ModelEntity_h
#define __smtk_view_ModelEntity_h

#include "smtk/view/Base.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT ModelEntity : public Base
    {
    public:
      static smtk::view::ModelEntityPtr New(const std::string &myName)
      { return smtk::view::ModelEntityPtr(new ModelEntity(myName)); }

      ModelEntity(const std::string &myTitle);

      virtual ~ModelEntity();
      virtual Base::Type type() const;
      // NEED TO CLARIFY - are groups of entities
      // represented by this mask?  For example, a set of model faces??
      // If not we need to add something to clarify this
      smtk::model::BitFlags modelEntityMask() const
      {return this->m_modelEntityMask;}
      void setModelEntityMask(smtk::model::BitFlags mask)
      {this->m_modelEntityMask = mask;}

      // If this def is not null then the section should
      // display all model entities of the requested mask along
      // with the attribute of this type in a table view
      smtk::attribute::DefinitionPtr definition() const
      {return this->m_attributeDefinition;}
      void setDefinition(smtk::attribute::DefinitionPtr def)
      {this->m_attributeDefinition = def;}

    protected:
      smtk::model::BitFlags m_modelEntityMask;
      smtk::attribute::DefinitionPtr m_attributeDefinition;
    private:

    };
  }
}

#endif /* __smtk_view_ModelEntity_h */
