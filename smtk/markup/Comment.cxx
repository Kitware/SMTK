//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Comment.h"

namespace smtk
{
namespace markup
{

Comment::~Comment() = default;

void Comment::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;

  auto it = data.find("mime-type");
  if (it != data.end())
  {
    this->setMimetype(it->get<smtk::string::Token>());
  }
  it = data.find("data");
  if (it != data.end())
  {
    this->setData(it->get<smtk::string::Token>());
  }
}

bool Comment::setData(const smtk::string::Token& data)
{
  if (m_data == data)
  {
    return false;
  }
  m_data = data;
  return true;
}

const smtk::string::Token& Comment::data() const
{
  return m_data;
}

smtk::string::Token& Comment::data()
{
  return m_data;
}

bool Comment::setMimetype(const smtk::string::Token& mimetype)
{
  if (m_mimetype == mimetype)
  {
    return false;
  }
  m_mimetype = mimetype;
  return true;
}

const smtk::string::Token& Comment::mimetype() const
{
  return m_mimetype;
}

smtk::string::Token& Comment::mimetype()
{
  return m_mimetype;
}

} // namespace markup
} // namespace smtk
