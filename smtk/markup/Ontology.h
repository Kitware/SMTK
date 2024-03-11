//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Ontology_h
#define smtk_markup_Ontology_h

#include "smtk/markup/Component.h"

#include "smtk/markup/IdSpace.h"

namespace smtk
{
namespace markup
{

/// A collection of labels related to one another by relationships, typically imported from an OWL
class SMTKMARKUP_EXPORT Ontology : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::Ontology);
  smtkSuperclassMacro(smtk::markup::Component);

  using IdType = smtk::markup::IdSpace::IdType;

  template<typename... Args>
  Ontology(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~Ontology() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  bool setUrl(const std::string& url);
  const std::string& url() const;
  std::string& url();

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  std::string m_url;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Ontology_h
