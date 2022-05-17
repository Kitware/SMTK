//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_SubphraseGenerator_h
#define smtk_view_SubphraseGenerator_h

#include "smtk/view/DescriptivePhrase.h"

namespace smtk
{
namespace model
{
class Group;
}

namespace view
{

/**\brief Generate subphrases to display for a given descriptive phrase.
  *
  * This abstract class should be subclassed by user interfaces
  * to provide control over what information is presented about a
  * given entity or set of entities.
  *
  * Its subphrases() method takes in a single phrase and
  * returns an ordered array of child phrases.
  * If the input phrase is null, then the generator should return
  * an array of top-level phrases.
  *
  * A phrase _generator_ may hold a weak reference to a phrase _model_.
  * The generator's purpose is to compute a list of child phrases
  * while a model's purpose is to interface a phrase hierarchy to a
  * concrete user interface. This involves notifying observers of changes
  * to phrases (including insertions, deletions, moves, and modifications).
  * A model may also need to decorate phrase content produced by the generator
  * to accommodate different UI affordances.
  * For this reason, when using a phrase model, it should be set on
  * each subphrase generator in the hierarchy. Then, when a descriptive
  * phrase uses the generator to build a portion of the hierarchy, the model
  * can be informed of the changes.
  */
class SMTKCORE_EXPORT SubphraseGenerator : smtkEnableSharedPtr(SubphraseGenerator)
{
public:
  static std::string getType(const smtk::view::ConfigurationPtr& viewSpec);
  static SubphraseGeneratorPtr create(
    const std::string& typeName,
    const smtk::view::ManagerPtr& manager);

  smtkTypeMacroBase(smtk::view::SubphraseGenerator);
  smtkCreateMacro(smtk::view::SubphraseGenerator);
  SubphraseGenerator();
  virtual ~SubphraseGenerator() = default;

  using Path = std::vector<int>;
  using PhrasesByPath = std::multimap<Path, DescriptivePhrasePtr>;

  /**\brief Return a list of descriptive phrases that elaborate upon \a src.
    *
    * Subclasses must override this method.
    */
  virtual DescriptivePhrases subphrases(DescriptivePhrase::Ptr src);

  /**\brief Return true if children would be generated for the Descriptive Phrase.
   */
  virtual bool hasChildren(const DescriptivePhrase& src) const;

  /**\brief Return a set of parent Persistent Objects for this object.
   * based on the generator's parent/child rules
   */
  virtual smtk::resource::PersistentObjectSet parentObjects(
    const smtk::resource::PersistentObjectPtr& obj) const;

  /// Set the phrase model used to adapt phrases to a user interface.
  bool setModel(PhraseModelPtr model);

  /// Return the phrase model (if any) used to adapt phrases to a user interface.
  PhraseModelPtr model() const { return m_model.lock(); }

  /**\brief Create a new Subphrase for an object which will be a child of parent
   *  and return the path to the phrase.
   */
  virtual DescriptivePhrasePtr createSubPhrase(
    const smtk::resource::PersistentObjectPtr& obj,
    const DescriptivePhrasePtr& parent,
    Path& childPath);

  /**\brief Append subphrases and their paths that the given set of created objects implies.
    *
    * After an operation, newly-created objects (components and resources) need to be
    * inserted into the descriptive phrase hierarchy. Since the subphrase generator is
    * responsible for populating all of the tree except the top-level phrases initially,
    * this task also falls to the generator.
    *
    * The generator is responsible for decorating each path it adds to \a resultingPhrases
    * if a phrase model is present.
    */
  virtual void subphrasesForCreatedObjects(
    const smtk::resource::PersistentObjectArray& objects,
    const DescriptivePhrasePtr& localRoot,
    PhrasesByPath& resultingPhrases);

  /**\brief The maximum number of subphrases to directly include before turning into a list.
    *
    * The helper methods in SubphraseGenerator (such as InstancesOfEntity()), will
    * insert small numbers of child items directly into the \a result subphrases.
    * But when more than \a directLimit() items of a given type (such as instances
    * for which the given entity serves as a prototype) are present, a sublist is
    * added a subphrase. This prevents long lists from obscuring other details.
    *
    * Subclasses may override this method.
    */
  virtual int directLimit() const;
  /**\brief Set the maximum number of direct children before a summary phrase is inserted.
    *
    * This is used to add a layer of indirection to the hierarchy so that
    * long lists are not inadvertently opened and so that a parent which would
    * otherwise have many children of many different kinds can group its
    * children to allow easier browsing.
    *
    * A negative value indicates that no limit should be imposed (no summary
    * phrases will ever be generated).
    */
  virtual bool setDirectLimit(int val);

  /**\brief Should the property of the given type and name be omitted from presentation?
    *
    * Subclasses should override this method.
    */
  virtual bool shouldOmitProperty(
    DescriptivePhrase::Ptr parent,
    smtk::resource::PropertyType ptype,
    const std::string& pname) const;

  /**\brief Get/Set whether entity properties will be skiped for subphrases.
    *
    * For some cases, only model entities are desired in a hierarchy view.
    */
  ///@{
  virtual void setSkipProperties(bool val);
  virtual bool skipProperties() const;
  ///@}

  /**\brief Get/Set whether entity attributes will be skiped for subphrases.
    *
    * For some cases, only model entities are desired in a hierarchy view.
    */
  ///@{
  virtual void setSkipAttributes(bool val);
  virtual bool skipAttributes() const;
  ///@}

  /// SubphraseGenerators that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }
  friend Manager;

protected:
  virtual Path indexOfObjectInParent(
    const smtk::resource::PersistentObjectPtr& obj,
    const smtk::view::DescriptivePhrasePtr& parent,
    const Path& parentPath);

  virtual int findResourceLocation(
    smtk::resource::ResourcePtr rsrc,
    const DescriptivePhrase::Ptr& root) const;
  virtual bool findSortedLocation(
    Path& pathOut,
    smtk::attribute::AttributePtr attr,
    DescriptivePhrase::Ptr& phr,
    const DescriptivePhrase::Ptr& parent) const;
  virtual bool findSortedLocation(
    Path& pathOut,
    smtk::model::EntityPtr entity,
    DescriptivePhrase::Ptr& phr,
    const DescriptivePhrase::Ptr& parent) const;
  virtual bool findSortedLocation(
    Path& pathOut,
    smtk::mesh::ComponentPtr comp,
    DescriptivePhrase::Ptr& phr,
    const DescriptivePhrase::Ptr& parent) const;

  /// Return true if the resource would cause subphrases to be generated
  bool resourceHasChildren(const smtk::resource::ResourcePtr& rsrc) const;
  /// Return true if the model entity would cause subphrases to be generated
  bool modelEntityHasChildren(const smtk::model::EntityPtr& entity) const;

  /// Populate \a result with the top-level components of \a rsrc with \a src as their parent.
  void componentsOfResource(
    DescriptivePhrase::Ptr src,
    smtk::resource::ResourcePtr rsrc,
    DescriptivePhrases& result);
  /// Populate \a result with the active, public items of \a att with \a src as their parent.
  void itemsOfAttribute(
    DescriptivePhrase::Ptr src,
    smtk::attribute::AttributePtr att,
    DescriptivePhrases& result);
  /// Populate \a result with the children of \a modelEntity with \a src as their parent.
  void childrenOfModelEntity(
    DescriptivePhrase::Ptr src,
    smtk::model::EntityPtr modelEntity,
    DescriptivePhrases& result);

  /** \brief Model-entity utility methods.
    *
    * These methods are called by childrenOfModelEntity and are available
    * for subclasses dealing with presenting the model tree.
    */
  ///@{
  /// Add submodels of \a mod to \a result with \a src as their parent.
  void freeSubmodelsOfModel(
    DescriptivePhrase::Ptr src,
    const smtk::model::Model& mod,
    DescriptivePhrases& result);
  /// Add groups of \a mod to \a result with \a src as their parent.
  void freeGroupsOfModel(
    DescriptivePhrase::Ptr src,
    const smtk::model::Model& mod,
    DescriptivePhrases& result);
  /// Add free cells of \a mod to \a result with \a src as their parent.
  void freeCellsOfModel(
    DescriptivePhrase::Ptr src,
    const smtk::model::Model& mod,
    DescriptivePhrases& result);
  /// Add auxiliary geometry entities of \a mod to \a result with \a src as their parent.
  void freeAuxiliaryGeometriesOfModel(
    DescriptivePhrase::Ptr src,
    const smtk::model::Model& mod,
    DescriptivePhrases& result);

  /// Add instances of \a ent to \a result with \a src as their parent.
  void instancesOfModelEntity(
    DescriptivePhrase::Ptr src,
    const smtk::model::EntityRef& ent,
    DescriptivePhrases& result);
  /// Add attributes associated with \a ent to \a result with \a src as their parent.
  /*
  void attributesOfModelEntity(
    DescriptivePhrase::Ptr src, const smtk::model::EntityRef& ent, DescriptivePhrases& result);
    */

  void cellOfModelUse(
    DescriptivePhrase::Ptr src,
    const smtk::model::UseEntity& ent,
    DescriptivePhrases& result);
  void boundingShellsOfModelUse(
    DescriptivePhrase::Ptr src,
    const smtk::model::UseEntity& ent,
    DescriptivePhrases& result);
  void toplevelShellsOfModelUse(
    DescriptivePhrase::Ptr src,
    const smtk::model::UseEntity& ent,
    DescriptivePhrases& result);

  void usesOfModelCell(
    DescriptivePhrase::Ptr src,
    const smtk::model::CellEntity& ent,
    DescriptivePhrases& result);
  void inclusionsOfModelCell(
    DescriptivePhrase::Ptr src,
    const smtk::model::CellEntity& ent,
    DescriptivePhrases& result);
  void boundingCellsOfModelCell(
    DescriptivePhrase::Ptr src,
    const smtk::model::CellEntity& ent,
    DescriptivePhrases& result);

  void usesOfModelShell(
    DescriptivePhrase::Ptr src,
    const smtk::model::ShellEntity& ent,
    DescriptivePhrases& result);

  void membersOfModelGroup(
    DescriptivePhrase::Ptr src,
    const smtk::model::Group& grp,
    DescriptivePhrases& result);

  void childrenOfModelAuxiliaryGeometry(
    DescriptivePhrase::Ptr src,
    const smtk::model::AuxiliaryGeometry& aux,
    DescriptivePhrases& result);

  void prototypeOfModelInstance(
    DescriptivePhrase::Ptr src,
    const smtk::model::Instance& ent,
    DescriptivePhrases& result);

  template<typename T>
  void PreparePath(T& result, const T& parentPath, int childIndex);

  template<typename T>
  int IndexFromTitle(const std::string& title, const T& phrases);

  ///@}

  /**\brief A templated helper for creating lists of components as subphrases.
    *
    * This method will create a new descriptive phrase with no PhraseContent and
    * its type set to COMPONENT_LIST.
    * That phrase will added to \a result.
    * It will have its subphrases populated with new phrases, one per entry of
    * the \a components container, each with ComponentPhraseContent.
    * If a comparator is passed in, then the children of this new phrase will be
    * sorted using the comparator.
    */
  template<typename T>
  PhraseListContentPtr addComponentPhrases(
    const T& components,
    DescriptivePhrase::Ptr parent,
    DescriptivePhrases& result,
    int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR),
    std::function<bool(const DescriptivePhrase::Ptr&, const DescriptivePhrase::Ptr&)> comparator =
      DescriptivePhrase::compareByTypeThenTitle);

  /**\brief A template helper for filtering the model entity phrase candidates
   * As a model entity could be marked as excluded from view presentation, this helper will
   * update the container to respect this property.
   * Assumption: Type T is std::vector holding smtk::model::EntityRef or its subclass type.
   */
  template<typename T>
  void filterModelEntityPhraseCandidates(T& ents);

  /// A templated helper for the model-related utility methods.
  template<typename T>
  PhraseListContentPtr addModelEntityPhrases(
    const T& ents,
    DescriptivePhrase::Ptr parent,
    int limit,
    DescriptivePhrases& result,
    int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR),
    std::function<bool(const DescriptivePhrase::Ptr&, const DescriptivePhrase::Ptr&)> comparator =
      nullptr);

#if 0
  /**\brief Property phrases.
    *
    * Components of resources that inherit ResourceWithProperties may wish to expose those
    * properties to users.
    * These convenience methods add properties of the given component as child phrases
    * of the given \a src.
    */
  ///@{
  /// Add all properties (of any type) as children.
  void propertiesOfComponent(
    DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result);
  /// Add only floating-point properties as children.
  void floatPropertiesOfComponent(
    DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result);
  /// Add only string properties as children.
  void stringPropertiesOfComponent(
    DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result);
  /// Add only integer properties as children.
  void integerPropertiesOfComponent(
    DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result);
  ///@}

  void modelsOfModelSession(
    DescriptivePhrase::Ptr src, const SessionRef& sess, DescriptivePhrases& result);

  void meshesOfModelModel(DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);
  void meshsetsOfMesh(MeshPhrase::Ptr meshphr, DescriptivePhrases& result);
  void meshesOfMeshList(MeshListPhrase::Ptr src, DescriptivePhrases& result);
  void meshsetsOfCollectionByDim(
    MeshPhrase::Ptr meshphr, smtk::mesh::DimensionType dim, DescriptivePhrases& result);

  void addEntityProperties(smtk::resource::PropertyType ptype, std::set<std::string>& props,
    DescriptivePhrase::Ptr parent, DescriptivePhrases& result);

  template <typename T>
  void addMeshPhrases(
    const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result);
#endif // 0

  int m_directLimit;
  bool m_skipAttributes;
  bool m_skipProperties;
  WeakPhraseModelPtr m_model;
  WeakManagerPtr m_manager;
};

} // namespace view
} // namespace smtk

#endif
