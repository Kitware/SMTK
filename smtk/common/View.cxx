//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkView.cxx - Base class for SMTK views
// .SECTION Description
// .SECTION See Also


#include "smtk/common/View.h"
#include <sstream>      // std::istringstream
#include <algorithm>

namespace smtk {
  namespace common {

//----------------------------------------------------------------------------
    View::Component &
    View::Component::setContents(const std::string &c)
        {
          this->m_contents = c;
          return *this;
        }
//----------------------------------------------------------------------------
    bool View::Component::attributeAsBool(const std::string &name, bool &val) const
    {
      std::string s;
      if (!this->attribute(name, s))
        {
        return false; // name doesn't exist
        }
      
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
      if ((s == "true") || (s == "t"))
        {
        val = true;
        return true;
        }
      
      if ((s == "false") || (s == "f"))
        {
        val = false;
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    bool View::Component::contentsAsInt(int &val) const
    {
      std::istringstream iss(this->m_contents);
      char c;
      int i;
      iss >> i;
      if (!iss.good())
          {
          return false;
          }
      return true;
    }
     
//----------------------------------------------------------------------------
    bool View::Component::contentsAsVector(std::vector<double> &vec) const
    {
      std::istringstream iss(this->m_contents);
      char c;
      double d;
      vec.clear();
      while (!iss.eof())
        {
        iss.get(c);
        if ((c == ' ') || (c == ','))
          {
          continue;
          }
        iss.putback(c);
        iss >> d;
        if (!iss.good())
          {
          return (vec.size() > 0);
          }
        vec.push_back(d);
        }
      return (vec.size() > 0);
    }


//----------------------------------------------------------------------------
    bool View::Component::attribute(const std::string &name, std::string &value) const
    {
      std::map<std::string, std::string>::const_iterator it;
      it = this->m_attributes.find(name);
      if (it == this->m_attributes.end())
        {
        return false;
        }
      value = it->second;
      return true;
    }

//----------------------------------------------------------------------------
    View::Component &
    View::Component::setAttribute(const std::string &name, const std::string &value)
        {
          this->m_attributes[name] = value;
          return *this;
        }
//----------------------------------------------------------------------------
    View::Component &View::Component::addChild(const std::string &childName)
    {
      this->m_children.push_back(Component(childName));
      return this->m_children.back();
    }
//----------------------------------------------------------------------------
    int View::Component::findChild(const std::string &compName) const
    {
      std::size_t i, n = this->m_children.size();
      for (i = 0; i < n; i++)
        {
        if (this->m_children[i].name() == compName)
          {
          return i;
          }
        }
      return -1;
    }
    
//----------------------------------------------------------------------------
    View::View(const std::string &myType, const std::string &myTitle):
      m_title(myTitle), m_type(myType), m_details("Details")
    {
    }

//----------------------------------------------------------------------------
    View::~View()
    {
    }

//----------------------------------------------------------------------------
  } // namespace common
} // namespace smtk
