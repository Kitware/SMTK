//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_OntologyIdentifier_h
#define smtk_markup_OntologyIdentifier_h

#include "smtk/markup/Label.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

class Ontology;
namespace arcs
{
struct OntologyIdentifiersToIndividuals;
struct OntologyToIdentifiers;
} // namespace arcs

/**\brief An entry in an ontology used to label components.
  *
  * Instances of this class _must_ have an arc relating them to the
  * ontology to which they belong. They _may_ have any number of arcs
  * connecting them to components, indicating an "is a" relationship
  * (i.e., the component is an instance of the ontology ID).
  *
  * For example, you may label an smtk::markup::UnstructuredData node with
  * an OntologyIdentifier node whose ontologyId() is
  * "http://purl.obolibrary.org/obo/UBERON_0000981", indicating the
  * geometry represents a femur bone. The OntologyIdentifier in turn
  * is connected to an Ontology node representing the Uberon anatomical
  * database as well as other OntologyIdentifier nodes such as
  * "http://purl.obolibrary.org/obo/UBERON_0003608", which is the
  * ID for "hindlimb long bones" (the base type of femur bones).
  */
class SMTKMARKUP_EXPORT OntologyIdentifier : public smtk::markup::Label
{
public:
  smtkTypeMacro(smtk::markup::OntologyIdentifier);
  smtkSuperclassMacro(smtk::markup::Label);

  template<typename... Args>
  OntologyIdentifier(Args&&... args)
    : smtk::markup::Label(std::forward<Args>(args)...)
  {
  }

  ~OntologyIdentifier() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /// A URL in the ontologys schema that can be queried for relationships.
  bool setOntologyId(const smtk::string::Token& ontologyId);
  const smtk::string::Token& ontologyId() const;
  smtk::string::Token& ontologyId();

  /// A convenience method to fetch the parent ontology (using the arc API).
  Ontology* ontology() const;

  /// Arc to components tagged with this ontology identifier.
  ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, ConstArc, OutgoingArc> subjects()
    const;
  ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, NonConstArc, OutgoingArc> subjects();

  /// Arc to the parent ontology of this identifer
  ArcEndpointInterface<arcs::OntologyToIdentifiers, ConstArc, IncomingArc> parent() const;
  ArcEndpointInterface<arcs::OntologyToIdentifiers, NonConstArc, IncomingArc> parent();

protected:
  smtk::string::Token m_ontologyId; // TODO: replace with a property rather than local storage.
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_OntologyIdentifier_h
