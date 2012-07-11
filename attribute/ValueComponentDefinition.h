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
// .NAME ValueComponentDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ValueComponentDefinition_h
#define __slctk_attribute_ValueComponentDefinition_h

#include <string>
#include <set>

class slctk::attribute::Attribute;
class slctk::attribute::Cluster;
class slctk::attribute::ComponentDefintion;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT ValueComponentDefinition : 
      public slctk::attribute::ComponentDefinition
    {
    public:
      bool isDiscrete() const
      {return (this->m_discreteValueLables.size() != 0);}
      void setIsDiscrete(bool isDiscreteValue)
      {this->m_isDiscrete = isDiscreteValue;}
      
      int defaultDiscreteIndex() const
      {return this->m_defaultDiscreteIndex;}
      void setDefaultDiscreteIndex(int discreteIndex)
      {this->m_defaultDiscreteIndex = discreteIndex;}
      
      bool hasDefault() const
      {return this->m_hasDefault;}

      virtual bool hasRange() const = 0;

      int numberOfValues() const
      {return this->m_numberOfElements;}
      void setNumberOfValues(int esize)
      {this->m_numberOfElements = esize;

      bool hasValueLables() const
      {return this->m_elementLabels.size();}

      void setValueLabel(int element, const std::string &elabel);
      void setCommonValueLable(const std::string &elable);
      const std::string &valueLable(int element) const;

    protected:
      // ValueComponentDefinitions can only be created by an attribute manager
      ValueComponentDefinition(const std::string &myname, 
                               unsigned long myId);
      virtual ~ValueComponentDefinition();

      bool m_hasDefault;
      bool m_useCommonLabel;
      std::vector<std::string> m_elementLables;
      std::vector<std::string> m_discreteValueLables;
      int m_defaultDiscreteIndex;
      int m_numberOfElements;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_ValueComponentDefinition_h */
