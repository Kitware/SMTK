//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Base.h - represents the base class for views
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Base_h
#define __smtk_view_Base_h
#include "smtk/CoreExports.h"
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
      void setUserData(const std::string &key, smtk::simulation::UserDataPtr value)
      {this->m_userData[key] = value;}
      smtk::simulation::UserDataPtr userData(const std::string &key) const;
      void clearUserData(const std::string &key)
      {this->m_userData.erase(key);}
      void clearAllUserData()
      {this->m_userData.clear();}

      static std::string type2String(Base::Type t);
      static Base::Type string2Type(const std::string &s);

    protected:
      std::map<std::string, smtk::simulation::UserDataPtr > m_userData;
      std::string m_title;
      std::string m_iconName;
    private:

    };
//----------------------------------------------------------------------------
    inline smtk::simulation::UserDataPtr Base::userData(const std::string &key) const
    {
      std::map<std::string, smtk::simulation::UserDataPtr >::const_iterator it =
        this->m_userData.find(key);
      return ((it == this->m_userData.end()) ? smtk::simulation::UserDataPtr() : it->second);
    }

  }
}

#endif /* __smtk_view_Base_h */
