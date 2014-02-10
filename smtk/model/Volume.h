#ifndef __smtk_model_Volume_h
#define __smtk_model_Volume_h

#include "smtk/model/CellEntity.h"
#include "smtk/model/VolumeUse.h"

#include <vector>

namespace smtk {
  namespace model {

class Face;
class Shell;
typedef std::vector<Face> Faces;
typedef std::vector<Shell> Shells;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Volume : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Volume,CellEntity,isVolume);

  VolumeUse use() const;
  Shells shells() const;
  Faces faces() const;

  Volume& setVolumeUse(const VolumeUse& volUse);
};

typedef std::vector<Volume> Volumes;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Volume_h
