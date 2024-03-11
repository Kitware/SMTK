//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Ontology.h"

namespace smtk
{
namespace markup
{

Ontology::~Ontology() = default;

void Ontology::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

bool Ontology::setUrl(const std::string& url)
{
  if (m_url == url)
  {
    return false;
  }
  m_url = url;
  return true;
}

const std::string& Ontology::url() const
{
  return m_url;
}

std::string& Ontology::url()
{
  return m_url;
}

bool Ontology::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourceOntology = std::dynamic_pointer_cast<const Ontology>(source))
  {
    this->setUrl(sourceOntology->url());
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
