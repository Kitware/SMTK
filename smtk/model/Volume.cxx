#include "smtk/model/Volume.h"

#include "smtk/model/Face.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

smtk::model::Faces Volume::faces() const
{
  Faces result;
  Cursors all = this->bordantEntities(/*dim = */ 2);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isFace())
      result.push_back(*it);
    }
  return result;
}

  } // namespace model
} // namespace smtk
