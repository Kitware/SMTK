#ifndef FBIGEOM_MOAB_HPP
#define FBIGEOM_MOAB_HPP

#include "FBiGeom.h"
//#include "moab/Forward.hpp"
#include "moab/Interface.hpp"
#include "moab/FBEngine.hpp"
#include "iMesh.h"
#include "MBiMesh.hpp"

/* map from MOAB's MBErrorCode to tstt's */
extern "C" const iBase_ErrorType iBase_ERROR_MAP[moab::MB_FAILURE+1];

// the igeom moab instance should privide easy access to
// moab::Interface, FBEngine *, and equivalent MBiMesh instance, because a lot
// of code can be shared among iMesh and iGeom, especially
// with respect to tags and sets
// when a moab iGeom is instanced, moab will be instanced, and FBEngine too
//
class MBiGeom
{
  MBiMesh * _mbimesh;
  moab::FBEngine * _fbe;
  bool _mbimeshCreated, _fbeCreated;
public:
  MBiGeom ()
  {
    // this will instance a moab Core, too
    _mbimesh = new MBiMesh(NULL);
    moab::Interface * mbi = _mbimesh->mbImpl;
    // pass mbi, so they will point to the same implementation
    _fbe = new FBEngine(mbi);
    _mbimeshCreated = _fbeCreated = true;
  }
  MBiGeom (MBiMesh * mbi, moab::FBEngine * fbe)
  {
    _mbimesh = mbi;
    _fbe = fbe;
    _mbimeshCreated = _fbeCreated = false;
  }
  ~MBiGeom()
  {
    // some cleanup here
    if (_fbeCreated) delete _fbe;
    if (_mbimeshCreated) delete _mbimesh;
  }
  moab::Interface * moabItf() { return _mbimesh->mbImpl;}
  moab::FBEngine * FBItf() { return _fbe;}
  MBiMesh * mbimesh() { return _mbimesh; }
};
/* Define macro for quick reference to MBInterface instance */
static inline moab::Interface* MBI_cast( FBiGeom_Instance i )  
  { return reinterpret_cast<MBiGeom*>(i)->moabItf(); }

#define MBI MBI_cast(instance)

static inline moab::FBEngine* FBE_cast( FBiGeom_Instance i )
  { return reinterpret_cast<MBiGeom*>(i) -> FBItf(); }

/* Define macro for quick reference to moab::Interface instance */
static inline moab::EntityHandle MBH_cast( iBase_EntityHandle h )  
  { return reinterpret_cast<moab::EntityHandle>(h); }         

#define GETGTT(a) (reinterpret_cast<MBiGeom*>(a)->FBItf()->get_gtt())

static inline bool FBiGeom_isError(int code)
  { return (iBase_SUCCESS != code); }
static inline bool FBiGeom_isError(moab::ErrorCode code)
  { return (moab::MB_SUCCESS != code); }

// easy access to imesh instance, used for tags, sets methods
#define IMESH_INSTANCE(i) reinterpret_cast<iMesh_Instance>( reinterpret_cast<MBiGeom*>(i)->mbimesh() )

// this assumes that iGeom instance is always instance
// uses MBiGeom class which sets the error
#define MBIM reinterpret_cast<MBiGeom*>(instance)->mbimesh()

#define RETURN(CODE)                                                   \
  do {                                                                 \
    *err = MBIM->set_last_error((CODE), "");                       \
    return;                                                            \
  } while(false)

#define ERROR(CODE,MSG)                                                \
  do {                                                                 \
    *err = MBIM->set_last_error((CODE), (MSG));                    \
    return;                                                            \
  } while(false)

#define CHKERR(CODE,MSG)                                               \
  do {                                                                 \
    if (FBiGeom_isError((CODE)))                                       \
      ERROR((CODE),(MSG));                                             \
  } while(false)

#define FWDERR()                                                       \
  do {                                                                 \
    if (FBiGeom_isError(*err))                                         \
      return;                                                          \
  } while(false)

#define CHECK_SIZE(array, allocated, size, type, retval)               \
  do {                                                                 \
    if (0 != allocated && NULL != array && allocated < (size)) {       \
      ERROR(iBase_MEMORY_ALLOCATION_FAILED, "Allocated array not "     \
            "enough to hold returned contents.");                      \
    }                                                                  \
    if ((size) && ((allocated) == 0 || NULL == (array))) {             \
      array = (type*)malloc((size)*sizeof(type));                      \
      allocated=(size);                                                \
      if (NULL == array) {                                             \
        ERROR(iBase_MEMORY_ALLOCATION_FAILED,                          \
              "Couldn't allocate array.");                             \
      }                                                                \
    }                                                                  \
  } while(false)

#endif // FBIGEOM_MOAB_HPP
