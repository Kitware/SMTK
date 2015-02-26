//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME MeshSelectionItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_MeshSelectionItem_h
#define __smtk_attribute_MeshSelectionItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class MeshSelectionItemDefinition;
    class SMTKCORE_EXPORT MeshSelectionItem : public Item
    {
    friend class MeshSelectionItemDefinition;
    public:
      smtkTypeMacro(MeshSelectionItem);
      virtual ~MeshSelectionItem();
      virtual Item::Type type() const;

  /// Enumeration of mesh values modification type.
  enum MeshSelectionMode {
    NONE            ,//!< Cancel current operation mode
    RESET           , //!< Reset the existing list)
    MERGE           , //!< Append to the existing list
    SUBTRACT        , //!< Subtract from existing list
    ACCEPT            //!< Accept the existing list)
  };

      void setValues(const std::vector<int>&);
      void appendValues(const std::vector<int>&);
      void removeValues(const std::vector<int>&);
      void setMeshSelectMode(MeshSelectionMode mode)
      { this->m_selectMode = mode; }
      MeshSelectionMode meshSelectMode() const
      {return this->m_selectMode;}
      void setCtrlKeyDown(bool val)
      { this->m_isCtrlKeyDown = val; }
      bool isCtrlKeyDown() const
      {return this->m_isCtrlKeyDown;}
  
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      int value(std::size_t element=0) const;
      bool appendValue(const int &val);
      bool removeValue(const int &val);
      virtual void reset();
      virtual std::string valueAsString() const
      {return this->valueAsString(0);}
      virtual std::string valueAsString(std::size_t element) const;
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);

      std::vector<int>::const_iterator begin() const;
      std::vector<int>::const_iterator end() const;

    protected:
      MeshSelectionItem(Attribute *owningAttribute, int itemPosition);
      MeshSelectionItem(Item *owningItem, int position, int subGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      std::vector<int>m_values;
      MeshSelectionMode m_selectMode;
      bool m_isCtrlKeyDown;
    private:
    };
  }
}


#endif /* __smtk_attribute_MeshSelectionItem_h */
