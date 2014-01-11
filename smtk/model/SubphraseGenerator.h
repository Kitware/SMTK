#ifndef __smtk_model_SubphraseGenerator_h
#define __smtk_model_SubphraseGenerator_h

#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"

#include "smtk/model/BRepModel.h" // For PropertyType enum.

namespace smtk {
  namespace model {

class InstanceEntity;

class DescriptivePhrase;
typedef std::vector<DescriptivePhrase> DescriptivePhrasees;

/**\brief Generate subphrases to display for a given descriptive phrase.
  *
  * This abstract class should be subclassed by user interfaces
  * to provide control over what information is presented about a
  * given entity or set of entities.
  *
  * Its subphrases() method takes in a single phrase and
  * returns an ordered array of phrases.
  *
  * Besides this one pure virtual method, some utility routines
  * are provided to fetch phrases for common information.
  * Subclasses may use these in their implementations of subphrases().
  */
class SMTKCORE_EXPORT SubphraseGenerator : smtkEnableSharedPtr(SubphraseGenerator)
{
public:
  smtkTypeMacro(SubphraseGenerator);

  virtual DescriptivePhrases subphrases(DescriptivePhrase::Ptr src) = 0;
  virtual int directLimit() const { return 4; }

protected:
  void InstancesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);
  void AttributesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);
  void PropertiesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);
  void FloatPropertiesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);
  void StringPropertiesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);
  void IntegerPropertiesOfEntity(DescriptivePhrase::Ptr src, const Cursor& ent, DescriptivePhrases& result);

  void CellOfUse(DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);
  void BoundingShellsOfUse(DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);
  void ToplevelShellsOfUse(DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);

  void ToplevelShellsOfCell(DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);
  void UsesOfCell(DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);
  void InclusionsOfCell(DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);
  void BoundingCellsOfCell(DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);

  void UsesOfShell(DescriptivePhrase::Ptr src, const ShellEntity& ent, DescriptivePhrases& result);

  void MembersOfGroup(DescriptivePhrase::Ptr src, const GroupEntity& grp, DescriptivePhrases& result);

  void FreeSubmodelsOfModel(DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result);
  void FreeGroupsInModel(DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result);
  void FreeCellsOfModel(DescriptivePhrase::Ptr src, const ModelEntity& mod, DescriptivePhrases& result);

  void PrototypeOfInstance(DescriptivePhrase::Ptr src, const InstanceEntity& ent, DescriptivePhrases& result);

  void EntitiesOfEntityList(EntityListPhrase::Ptr src, const CursorArray& ents, DescriptivePhrases& result);
  void PropertiesOfPropertyList(PropertyListPhrase::Ptr src, PropertyType p, DescriptivePhrases& result);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_SubphraseGenerator_h
