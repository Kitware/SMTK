#define iRel_Instance integer
#define iRel_PairHandle integer

      integer iRel_IGEOM_IFACE
      integer iRel_IMESH_IFACE
      integer iRel_IFIELD_IFACE
      integer iRel_IREL_IFACE
      integer iRel_FBIGEOM_IFACE

      parameter (iRel_IGEOM_IFACE = 0)
      parameter (iRel_IMESH_IFACE = 1)
      parameter (iRel_IFIELD_IFACE = 2)
      parameter (iRel_IREL_IFACE = 3)
      parameter (iRel_FBIGEOM_IFACE = 3)

      integer iRel_ENTITY
      integer iRel_SET
      integer iRel_BOTH

      parameter (iRel_ENTITY = 0)
      parameter (iRel_SET = 1)
      parameter (iRel_BOTH = 2)

      integer iRel_ACTIVE
      integer iRel_INACTIVE
      integer iRel_NOTEXIST

      parameter (iRel_ACTIVE = 0)
      parameter (iRel_INACTIVE = 1)
      parameter (iRel_NOTEXIST = 2)
