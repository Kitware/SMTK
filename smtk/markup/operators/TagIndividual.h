//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_TagIndividual_h
#define smtk_markup_TagIndividual_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

#include "smtk/attribute/ComponentItem.h"

namespace smtk
{
namespace markup
{

/// Create an ontology node that tags the associated objects as individual
/// instances of a class.
class SMTKMARKUP_EXPORT TagIndividual : public smtk::operation::XMLOperation
{

public:
  smtkTypeMacro(smtk::markup::TagIndividual);
  smtkCreateMacro(TagIndividual);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;

  /// Find or create an OntologyIdentifier with the given \a nodeName,
  /// \a ontologyName, and \a nodeURL.
  ///
  /// This may also create an Ontology node. If it does, it will be
  /// added to \a created, as will the identifier (if created rather
  /// than found).
  smtk::markup::OntologyIdentifier::Ptr findOrCreateTag(
    smtk::markup::Resource* resource,
    const std::string& nodeName,
    const std::string& ontologyName,
    const std::string& nodeURL,
    const smtk::attribute::ComponentItem::Ptr& created,
    const smtk::attribute::ComponentItem::Ptr& modified,
    bool& createdIdentifier);

  /// For each object in \a assocs, connect it to \a tag and append it
  /// to the \a modified values.
  ///
  /// This returns the number of entries in \a assocs that were tagged.
  std::size_t tagNodes(
    const smtk::markup::OntologyIdentifier::Ptr& tag,
    const smtk::attribute::ReferenceItem::Ptr& assocs,
    const smtk::attribute::ComponentItem::Ptr& modified);

  /// For each object in \a assocs, disconnect it from \a tag and append
  /// it to the \a modified nodes. If \a tag is unused after this, remove
  /// it from the resource (via a Delete operation) and add the \a tag to
  /// the \a expunged item.
  ///
  /// This returns the number of entries in \a assocs that were untagged.
  std::size_t untagNodes(
    const smtk::markup::OntologyIdentifier::Ptr& tag,
    const smtk::attribute::ReferenceItem::Ptr& assocs,
    const smtk::attribute::ComponentItem::Ptr& modified,
    const smtk::attribute::ComponentItem::Ptr& expunged);

  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_TagIndividual_h
