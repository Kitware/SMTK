//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME InstancedSection.h - Groups individual/singleton attributes in the GUI
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Instanced_h
#define __smtk_view_Instanced_h

#include "smtk/view/Base.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <vector>

//NOTE THAT WE ASSUME THAT THE READER Takes care of creating the instances if they are
// not found!!
namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Instanced : public Base
    {
    public:
      static smtk::view::InstancedPtr New(const std::string &myName)
      { return smtk::view::InstancedPtr(new Instanced(myName)); }

      Instanced(const std::string &myTitle);
      virtual ~Instanced();
      virtual Base::Type type() const;
      void addInstance(smtk::attribute::AttributePtr att)
        { if(att.get() != NULL)
            {this->m_instances.push_back(att);}
        }
      std::size_t numberOfInstances() const
      {return this->m_instances.size();}
      smtk::attribute::AttributePtr instance(int ith) const
      {return this->m_instances[ith];}

    protected:
      std::vector<smtk::attribute::AttributePtr> m_instances;
    private:

    };
  }
}


#endif /* __smtk_view_Instanced_h */
