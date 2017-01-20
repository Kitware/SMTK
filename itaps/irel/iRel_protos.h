#ifndef IREL_PROTOS_H
#define IREL_PROTOS_H

#include "moab/MOABConfig.h"

#if defined(MOAB_FC_FUNC_)
#define ITAPS_FC_WRAPPER MOAB_FC_FUNC_
#elif defined(MOAB_FC_FUNC)
#define ITAPS_FC_WRAPPER MOAB_FC_FUNC
#else
#define ITAPS_FC_WRAPPER(name,NAME) name
#endif

#define iRel_getErrorType ITAPS_FC_WRAPPER( irel_geterrortype, IREL_GETERRORTYPE )
#define iRel_getDescription ITAPS_FC_WRAPPER( irel_getdescription, IREL_GETDESCRIPTION )
#define iRel_create ITAPS_FC_WRAPPER( irel_create, IREL_CREATE )
#define iRel_destroy ITAPS_FC_WRAPPER( irel_destroy, IREL_DESTROY )
#define iRel_createPair ITAPS_FC_WRAPPER( irel_createpair, IREL_CREATEPAIR )
#define iRel_getPairInfo ITAPS_FC_WRAPPER( irel_getpairinfo, IREL_GETPAIRINFO )
#define iRel_changePairType ITAPS_FC_WRAPPER( irel_changepairtype, IREL_CHANGEPAIRTYPE )
#define iRel_changePairStatus ITAPS_FC_WRAPPER( irel_changepairstatus, IREL_CHANGEPAIRSTATUS )
#define iRel_destroyPair ITAPS_FC_WRAPPER( irel_destroypair, IREL_DESTROYPAIR )
#define iRel_findPairs ITAPS_FC_WRAPPER( irel_findpairs, IREL_FINDPAIRS )
#define iRel_setEntEntRelation ITAPS_FC_WRAPPER( irel_setententrelation, IREL_SETENTENTRELATION )
#define iRel_setEntSetRelation ITAPS_FC_WRAPPER( irel_setentsetrelation, IREL_SETENTSETRELATION )
#define iRel_setSetEntRelation ITAPS_FC_WRAPPER( irel_setsetentrelation, IREL_SETSETENTRELATION )
#define iRel_setSetSetRelation ITAPS_FC_WRAPPER( irel_setsetsetrelation, IREL_SETSETSETRELATION )
#define iRel_setEntArrEntArrRelation ITAPS_FC_WRAPPER( irel_setentarrentarrrelation, IREL_SETENTARRENTARRRELATION )
#define iRel_setSetArrEntArrRelation ITAPS_FC_WRAPPER( irel_setsetarrentarrrelation, IREL_SETSETARRENTARRRELATION )
#define iRel_setEntArrSetArrRelation ITAPS_FC_WRAPPER( irel_setentarrsetarrrelation, IREL_SETENTARRSETARRRELATION )
#define iRel_setSetArrSetArrRelation ITAPS_FC_WRAPPER( irel_setsetarrsetarrrelation, IREL_SETSETARRSETARRRELATION )
#define iRel_getEntEntRelation ITAPS_FC_WRAPPER( irel_getententrelation, IREL_GETENTENTRELATION )
#define iRel_getEntSetRelation ITAPS_FC_WRAPPER( irel_getentsetrelation, IREL_GETENTSETRELATION )
#define iRel_getSetEntRelation ITAPS_FC_WRAPPER( irel_getsetentrelation, IREL_GETSETENTRELATION )
#define iRel_getSetSetRelation ITAPS_FC_WRAPPER( irel_getsetsetrelation, IREL_GETSETSETRELATION )
#define iRel_getEntSetIterRelation ITAPS_FC_WRAPPER( irel_getentsetiterrelation, IREL_GETENTSETITERRELATION )
#define iRel_getEntArrEntArrRelation ITAPS_FC_WRAPPER( irel_getentarrentarrrelation, IREL_GETENTARRENTARRRELATION )
#define iRel_getEntArrSetArrRelation ITAPS_FC_WRAPPER( irel_getentarrsetarrrelation, IREL_GETENTARRSETARRRELATION )
#define iRel_getSetArrEntArrRelation ITAPS_FC_WRAPPER( irel_getsetarrentarrrelation, IREL_GETSETARRENTARRRELATION )
#define iRel_getSetArrSetArrRelation ITAPS_FC_WRAPPER( irel_getsetarrsetarrrelation, IREL_GETSETARRSETARRRELATION )
#define iRel_getEntArrSetIterArrRelation ITAPS_FC_WRAPPER( irel_getentarrsetiterarrrelation, IREL_GETENTARRSETITERARRRELATION )
#define iRel_rmvEntRelation ITAPS_FC_WRAPPER( irel_rmventrelation, IREL_RMVENTRELATION )
#define iRel_rmvSetRelation ITAPS_FC_WRAPPER( irel_rmvsetrelation, IREL_RMVSETRELATION )
#define iRel_rmvEntArrRelation ITAPS_FC_WRAPPER( irel_rmventarrrelation, IREL_RMVENTARRRELATION )
#define iRel_rmvSetArrRelation ITAPS_FC_WRAPPER( irel_rmvsetarrrelation, IREL_RMVSETARRRELATION )
#define iRel_inferAllRelations ITAPS_FC_WRAPPER( irel_inferallrelations, IREL_INFERALLRELATIONS )
#define iRel_inferAllRelationsAndType ITAPS_FC_WRAPPER( irel_inferallrelationsandtype, IREL_INFERALLRELATIONSANDTYPE )
#define iRel_inferEntRelations ITAPS_FC_WRAPPER( irel_inferentrelations, IREL_INFERENTRELATIONS )
#define iRel_inferSetRelations ITAPS_FC_WRAPPER( irel_infersetrelations, IREL_INFERSETRELATIONS )
#define iRel_inferEntArrRelations ITAPS_FC_WRAPPER( irel_inferentarrrelations, IREL_INFERENTARRRELATIONS )
#define iRel_inferSetArrRelations ITAPS_FC_WRAPPER( irel_infersetarrrelations, IREL_INFERSETARRRELATIONS )

#endif
