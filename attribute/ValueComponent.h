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
// .NAME ValueComponent.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ValueComponent_h
#define __slctk_attribute_ValueComponent_h

#include "attribute/Component.h"
class slctk::attribute::ValueComponentDefinition;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT ValueComponent : public slctk::attribute::Component
    {
    public:
      size_type numberOfElements() const
      {return this->m_isSet.size();}

      int descreteIndex() const
      { return this->discreteIndex(0);}
      int descreteIndex(int element) const
      {return this->m_discreteIndex;}
      bool isDescrete() const;
      
      void setDiscreteIndex(int index)
      {this->setDiscreteIndex(0, index);}
      void setDiscreteIndex(int element, int index);
      // Reset returns the component to its initial state.
      //If the component is of fixed size, then it's values  to their initial state.  
      // If there is a default available it will use it, else
      // it will be marked as unset.
      //If the component's definition indicated a size of 0 then it will go back to 
      // having no values
      virtual reset() = 0;
      bool setToDefault(int element) const = 0;
      virtual const std::string &valueAsString(const std::string &format) const
      { return this->valueAsString(0, format);}

      virtual const std::string &valueAsString(int element, const std::string &format) const = 0;
      bool isSet() const
      { return this->m_isSet[0];}
      bool isSet(int element) const;
      {return this->m_isSet[element];}
      void unset() 
      {this->unset(0);}
      void unset(int element)
      {this->m_isSet[element] = false;}

    protected:
      ValueComponent(slctk::attribute::ValueComponentDefinition *def);
      virtual ~ValueComponent();
      virtual void updateDiscreteValue(int element) = 0;
      std::vector<int> m_discreteIndices;
      std::vector<bool> m_isSet;
      mutable std::string m_tempString;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_ValueComponent_h */
