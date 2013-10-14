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
// .NAME Base.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Base_h
#define __smtk_view_Base_h
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <string>
namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Base
    {
    public:
      enum Type
      {
        ATTRIBUTE,
        GROUP,
        INSTANCED,
        MODEL_ENTITY,
        ROOT,
        SIMPLE_EXPRESSION,
        NUMBER_OF_TYPES
      };

      Base(const std::string &myTitle);
      virtual ~Base();
      virtual Base::Type type() const = 0;
      std::string title() const
      { return this->m_title;}
      void setTitle(const std::string &myTitle)
        {this->m_title = myTitle;}
      std::string iconName() const
      { return this->m_iconName;}
      void setIconName(const std::string &myIcon)
        {this->m_iconName = myIcon;}
      void setUserData(const std::string &key, smtk::util::UserDataPtr value)
      {this->m_userData[key] = value;}
      smtk::util::UserDataPtr userData(const std::string &key) const;
      void clearUserData(const std::string &key)
      {this->m_userData.erase(key);}
      void clearAllUserData()
      {this->m_userData.clear();}

      static std::string type2String(Base::Type t);
      static Base::Type string2Type(const std::string &s);

    protected:
      std::map<std::string, smtk::util::UserDataPtr > m_userData;
      std::string m_title;
      std::string m_iconName;
    private:

    };
//----------------------------------------------------------------------------
    inline smtk::util::UserDataPtr Base::userData(const std::string &key) const
    {
      std::map<std::string, smtk::util::UserDataPtr >::const_iterator it =
        this->m_userData.find(key);
      return ((it == this->m_userData.end()) ? smtk::util::UserDataPtr() : it->second);
    }

  }
}

#endif /* __smtk_view_Base_h */
