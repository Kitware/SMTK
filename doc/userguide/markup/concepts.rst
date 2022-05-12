Key Concepts
============

The :smtk:`Markup Resource <smtk::markup::Resource>` is focused
on geometric annotations (such as selecting regions where boundary
conditions apply and identifying the spatial composition of materials).
In particular, it deals with discrete geometric models
(as opposed to smooth parametric models like the OpenCascade
resource) and annotations made on those models, which tend
not to be spatial or geometric in nature except for how they
reference other nodes' geometry.

Node types
^^^^^^^^^^

The nodes are arranged in an inheritance hierarchy:

:smtk:`Component <smtk::markup::Component>`
  is the base node type for all components in the markup resource.
  Several arc types (such as groups and labels) can apply to any
  node type, so there are methods here to provide easy access to
  any groups a component may be a member of or labels which apply
  to a component.

  :smtk:`SpatialData <smtk::markup::SpatialData>`
    is a base type for any component that has renderable geometry.
    The geometry maps from some abstract :smtk:`Domain <smtk::markup::Domain>`
    into world coordinates. These nodes may also be connected to
    Fields, which define functions over the space occupied by the node.

    :smtk:`UnstructuredData <smtk::markup::UnstructuredData>`
      represents components that have irregularly-connected cell primitives
      defined by "corner" points.

    :smtk:`ImageData <smtk::markup::ImageData>`
      represents components that have a regular lattice of cell primitives
      (called voxels).

    :smtk:`AnalyticShape <smtk::markup::AnalyticShape>`
      is a base class for components whose domain is an analytic function
      rather than a cell complex. Children include
      :smtk:`boxes <smtk::markup::Box>`, :smtk:`cones <smtk::markup::Cone>`,
      :smtk:`planes <smtk::markup::Plane>`, and
      :smtk:`generalized spheres <smtk::markup::Sphere>`.

  :smtk:`Field <smtk::markup::Field>`
    instances represent functions defined over SpatialData nodes.

  :smtk:`Label <smtk::markup::Label>`
    is a base class for information used to mark another node (i.e., to label it).
    The base class makes no assumption about the form of information used to
    mark up its subject node.

    :smtk:`Comment <smtk::markup::Comment>`
      is a label that holds flat text provided by the user.

    :smtk:`URL <smtk::markup::URL>`
      is a kind of label that references data on the internet via a uniform
      resource locator (URL).

    :smtk:`OntologyIdentifier <smtk::markup::OntologyIdentifier>`
      is a label that holds a specific URL labeling its subject components
      as being "named individuals" in a formal ontology.
      These labels can be used to perform automated inference of relationships
      to other nodes via their labels.

  :smtk:`Group <smtk::markup::Group>`
    is a node with arcs that denote membership. Any component can be placed
    into as many groups as desired. Groups may contain other groups.
    Deleting a group requires you to delete all of its members (i.e., the
    members prevent deletion of the group unless they, too, are deleted).
    However, *ungrouping* a group first removes its members before deleting
    the group.

  :smtk:`Ontology <smtk::markup::Ontology>`
    is a node that serves as the parent to a set of ontology identifiers
    and has a location indicating the source of the ontology identifier URLs.


Arc types
^^^^^^^^^

Arcs connect nodes to add contextual information to them

:smtk:`Boundaries to shapes <smtk::markup::arcs::BoundariesToShapes>`
  are arcs from side sets to the shapes on whose boundaries they lie.
  An example is the surface bounding a tetrahedral mesh.

:smtk:`Fields to shapes <smtk::markup::arcs::FieldsToShapes>`
  are arcs from functions to the shapes on which those functions are defined
  (i.e., their domains). An example is a label map defined on an image; an
  image may have many label maps (and other fields). Many fields are also
  defined over not just one shape but a collection of them (i.e., a simulation
  might compute deflection of many tissues, each modeled as its own component.

:smtk:`Groups to members <smtk::markup::arcs::GroupsToMembers>`
  are arcs from groups to their members.
  As discussed above, groups may contain other groups.
  A component can be contained in many groups.
  Components prevent their groups from being deleted.

:smtk:`Labels to subjects <smtk::markup::arcs::LabelsToSubjects>`
  are arcs from labels to the components they annotate.
  Since the label class is a base class for other, more specific, forms
  of information used to annotate components, this type of arc does not
  indicate the specific nature of the annotation. Other arcs below
  are used in contexts that require it.

:smtk:`Ontology identifiers to individuals <smtk::markup::arcs::BoundariesToShapes>`
  are arcs that indicate the target node is a "named individual" of the ontology class
  the identifier represents.
  Thus, where the "femur bone" ontology identifer is connected to nodes via this
  arc, those nodes are marked as instances of femur bones.

:smtk:`Ontology identifiers to sub-types <smtk::markup::arcs::OntologyIdentifiersToSubtypes>`
  are arcs from ontology identifers to other identifiers representing subclasses
  of the originating identifier.
  An example is a femur, which is a hindlimb long bone, which is a leg bone, and
  so forth. Each of these specializations are a sub-type.

:smtk:`Ontology to ontology identifiers <smtk::markup::arcs::OntologyToIdentifiers>`
  are arcs from side sets to the shapes on whose boundaries they lie.
  An example is the surface bounding a tetrahedral mesh.

:smtk:`References to primaries <smtk::markup::arcs::ReferencesToPrimaries>`
  are implicit arcs from any reference-based geometry (side sets or subsets)
  to the nodes which they reference (currently unstructured data or image data).
  Unlike the similar BoundariesToShapes arc type – which connect the totality
  of a boundary to the shape it bounds – these arcs include references to small,
  potentially overlapping, portions of boundaries (e.g., areas on bones serving
  as ligament insertion points).

:smtk:`URLs to data <smtk::markup::arcs::URLsToData>`
  are arcs from URLs where data is located to the data as loaded into memory.
  These URLs are files on disk containing geometric data as read (not imported)
  along with the nodes that reference them. A single file may contain data for
  many components; in that case many nodes will connect to a single URL node.

:smtk:`URLs to imported data <smtk::markup::arcs::URLsToImportedData>`
  are arcs from URLs where data was located at the time it was imported to
  the data as loaded into memory. This information is not needed to load a
  markup resource but serve as provenance information marking the source
  of the data.

.. todo:: Add a section domain types, especially IdSpace for discrete geometry.
