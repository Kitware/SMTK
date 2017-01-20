#ifndef IMESHP_PROTOS_H
#define IMESHP_PROTOS_H

#include "moab/MOABConfig.h"

#if defined(MOAB_FC_FUNC_)
#define ITAPS_FC_WRAPPER MOAB_FC_FUNC_
#elif defined(MOAB_FC_FUNC)
#define ITAPS_FC_WRAPPER MOAB_FC_FUNC
#else
#define ITAPS_FC_WRAPPER(name,NAME) name
#endif

#define iMeshP_createPartitionAll ITAPS_FC_WRAPPER( imeshp_createpartitionall, IMESHP_CREATEPARTITIONALL )
#define iMeshP_destroyPartitionAll ITAPS_FC_WRAPPER( imeshp_destroypartitionall, IMESHP_DESTROYPARTITIONALL )
#define iMeshP_getPartitionComm ITAPS_FC_WRAPPER( imeshp_getpartitioncomm, IMESHP_GETPARTITIONCOMM )
#define iMeshP_syncPartitionAll ITAPS_FC_WRAPPER( imeshp_syncpartitionall, IMESHP_SYNCPARTITIONALL )
#define iMeshP_getNumPartitions ITAPS_FC_WRAPPER( imeshp_getnumpartitions, IMESHP_GETNUMPARTITIONS )
#define iMeshP_getPartitions ITAPS_FC_WRAPPER( imeshp_getpartitions, IMESHP_GETPARTITIONS )
#define iMeshP_getNumGlobalParts ITAPS_FC_WRAPPER( imeshp_getnumglobalparts, IMESHP_GETNUMGLOBALPARTS )
#define iMeshP_getNumLocalParts ITAPS_FC_WRAPPER( imeshp_getnumlocalparts, IMESHP_GETNUMLOCALPARTS )
#define iMeshP_getLocalParts ITAPS_FC_WRAPPER( imeshp_getlocalparts, IMESHP_GETLOCALPARTS )
#define iMeshP_getRankOfPart ITAPS_FC_WRAPPER( imeshp_getrankofpart, IMESHP_GETRANKOFPART )
#define iMeshP_getRankOfPartArr ITAPS_FC_WRAPPER( imeshp_getrankofpartarr, IMESHP_GETRANKOFPARTARR )
#define iMeshP_getNumOfTypeAll ITAPS_FC_WRAPPER( imeshp_getnumoftypeall, IMESHP_GETNUMOFTYPEALL )
#define iMeshP_getNumOfTopoAll ITAPS_FC_WRAPPER( imeshp_getnumoftopoall, IMESHP_GETNUMOFTOPOALL )
#define iMeshP_createPart ITAPS_FC_WRAPPER( imeshp_createpart, IMESHP_CREATEPART )
#define iMeshP_destroyPart ITAPS_FC_WRAPPER( imeshp_destroypart, IMESHP_DESTROYPART )
#define iMeshP_getPartIdFromPartHandle ITAPS_FC_WRAPPER( imeshp_getpartidfromparthandle, IMESHP_GETPARTIDFROMPARTHANDLE )
#define iMeshP_getPartIdsFromPartHandlesArr ITAPS_FC_WRAPPER( imeshp_getpartidsfromparthandlesarr, IMESHP_GETPARTIDSFROMPARTHANDLESARR )
#define iMeshP_getPartHandleFromPartId ITAPS_FC_WRAPPER( imeshp_getparthandlefrompartid, IMESHP_GETPARTHANDLEFROMPARTID )
#define iMeshP_getPartHandlesFromPartsIdsArr ITAPS_FC_WRAPPER( imeshp_getparthandlesfrompartsidsarr, IMESHP_GETPARTHANDLESFROMPARTSIDSARR )
#define iMeshP_getNumPartNbors ITAPS_FC_WRAPPER( imeshp_getnumpartnbors, IMESHP_GETNUMPARTNBORS )
#define iMeshP_getNumPartNborsArr ITAPS_FC_WRAPPER( imeshp_getnumpartnborsarr, IMESHP_GETNUMPARTNBORSARR )
#define iMeshP_getPartNbors ITAPS_FC_WRAPPER( imeshp_getpartnbors, IMESHP_GETPARTNBORS )
#define iMeshP_getPartNborsArr ITAPS_FC_WRAPPER( imeshp_getpartnborsarr, IMESHP_GETPARTNBORSARR )
#define iMeshP_getNumPartBdryEnts ITAPS_FC_WRAPPER( imeshp_getnumpartbdryents, IMESHP_GETNUMPARTBDRYENTS )
#define iMeshP_getPartBdryEnts ITAPS_FC_WRAPPER( imeshp_getpartbdryents, IMESHP_GETPARTBDRYENTS )
#define iMeshP_initPartBdryEntIter ITAPS_FC_WRAPPER( imeshp_initpartbdryentiter, IMESHP_INITPARTBDRYENTITER )
#define iMeshP_initPartBdryEntArrIter ITAPS_FC_WRAPPER( imeshp_initpartbdryentarriter, IMESHP_INITPARTBDRYENTARRITER )
#define iMeshP_getNumOfType ITAPS_FC_WRAPPER( imeshp_getnumoftype, IMESHP_GETNUMOFTYPE )
#define iMeshP_getNumOfTopo ITAPS_FC_WRAPPER( imeshp_getnumoftopo, IMESHP_GETNUMOFTOPO )
#define iMeshP_getAdjEntIndices ITAPS_FC_WRAPPER( imeshp_getadjentindices, IMESHP_GETADJENTINDICES )
#define iMeshP_getEntities ITAPS_FC_WRAPPER( imeshp_getentities, IMESHP_GETENTITIES )
#define iMeshP_getAdjEntities ITAPS_FC_WRAPPER( imeshp_getadjentities, IMESHP_GETADJENTITIES )
#define iMeshP_initEntIter ITAPS_FC_WRAPPER( imeshp_initentiter, IMESHP_INITENTITER )
#define iMeshP_initEntArrIter ITAPS_FC_WRAPPER( imeshp_initentarriter, IMESHP_INITENTARRITER )
#define iMeshP_getEntOwnerPart ITAPS_FC_WRAPPER( imeshp_getentownerpart, IMESHP_GETENTOWNERPART )
#define iMeshP_getEntOwnerPartArr ITAPS_FC_WRAPPER( imeshp_getentownerpartarr, IMESHP_GETENTOWNERPARTARR )
#define iMeshP_isEntOwner ITAPS_FC_WRAPPER( imeshp_isentowner, IMESHP_ISENTOWNER )
#define iMeshP_isEntOwnerArr ITAPS_FC_WRAPPER( imeshp_isentownerarr, IMESHP_ISENTOWNERARR )
#define iMeshP_getEntStatus ITAPS_FC_WRAPPER( imeshp_getentstatus, IMESHP_GETENTSTATUS )
#define iMeshP_getEntStatusArr ITAPS_FC_WRAPPER( imeshp_getentstatusarr, IMESHP_GETENTSTATUSARR )
#define iMeshP_getNumCopies ITAPS_FC_WRAPPER( imeshp_getnumcopies, IMESHP_GETNUMCOPIES )
#define iMeshP_getCopyParts ITAPS_FC_WRAPPER( imeshp_getcopyparts, IMESHP_GETCOPYPARTS )
#define iMeshP_getCopies ITAPS_FC_WRAPPER( imeshp_getcopies, IMESHP_GETCOPIES )
#define iMeshP_getCopyOnPart ITAPS_FC_WRAPPER( imeshp_getcopyonpart, IMESHP_GETCOPYONPART )
#define iMeshP_getOwnerCopy ITAPS_FC_WRAPPER( imeshp_getownercopy, IMESHP_GETOWNERCOPY )
#define iMeshP_waitForRequest ITAPS_FC_WRAPPER( imeshp_waitforrequest, IMESHP_WAITFORREQUEST )
#define iMeshP_waitForAnyRequest ITAPS_FC_WRAPPER( imeshp_waitforanyrequest, IMESHP_WAITFORANYREQUEST )
#define iMeshP_waitForAllRequests ITAPS_FC_WRAPPER( imeshp_waitforallrequests, IMESHP_WAITFORALLREQUESTS )
#define iMeshP_waitForRequestEnt ITAPS_FC_WRAPPER( imeshp_waitforrequestent, IMESHP_WAITFORREQUESTENT )
#define iMeshP_testRequest ITAPS_FC_WRAPPER( imeshp_testrequest, IMESHP_TESTREQUEST )
#define iMeshP_pollForRequests ITAPS_FC_WRAPPER( imeshp_pollforrequests, IMESHP_POLLFORREQUESTS )
#define iMeshP_exchEntArrToPartsAll ITAPS_FC_WRAPPER( imeshp_exchentarrtopartsall, IMESHP_EXCHENTARRTOPARTSALL )
#define iMeshP_migrateEntity ITAPS_FC_WRAPPER( imeshp_migrateentity, IMESHP_MIGRATEENTITY )
#define iMeshP_updateVtxCoords ITAPS_FC_WRAPPER( imeshp_updatevtxcoords, IMESHP_UPDATEVTXCOORDS )
#define iMeshP_replaceOnPartBdry ITAPS_FC_WRAPPER( imeshp_replaceonpartbdry, IMESHP_REPLACEONPARTBDRY )
#define iMeshP_addGhostOf ITAPS_FC_WRAPPER( imeshp_addghostof, IMESHP_ADDGHOSTOF )
#define iMeshP_rmvGhostOf ITAPS_FC_WRAPPER( imeshp_rmvghostof, IMESHP_RMVGHOSTOF )
#define iMeshP_syncMeshAll ITAPS_FC_WRAPPER( imeshp_syncmeshall, IMESHP_SYNCMESHALL )
#define iMeshP_pushTags ITAPS_FC_WRAPPER( imeshp_pushtags, IMESHP_PUSHTAGS )
#define iMeshP_pushTagsEnt ITAPS_FC_WRAPPER( imeshp_pushtagsent, IMESHP_PUSHTAGSENT )
#define iMeshP_iPushTags ITAPS_FC_WRAPPER( imeshp_ipushtags, IMESHP_IPUSHTAGS )
#define iMeshP_iPushTagsEnt ITAPS_FC_WRAPPER( imeshp_ipushtagsent, IMESHP_IPUSHTAGSENT )
#define iMeshP_createGhostEntsAll ITAPS_FC_WRAPPER( imeshp_createghostentsall, IMESHP_CREATEGHOSTENTSALL )
#define iMeshP_deleteGhostEntsAll ITAPS_FC_WRAPPER( imeshp_deleteghostentsall, IMESHP_DELETEGHOSTENTSALL )
#define iMeshP_ghostEntInfo ITAPS_FC_WRAPPER( imeshp_ghostentinfo, IMESHP_GHOSTENTINFO )
#define iMeshP_loadAll ITAPS_FC_WRAPPER( imeshp_loadall, IMESHP_LOADALL )
#define iMeshP_saveAll ITAPS_FC_WRAPPER( imeshp_saveall, IMESHP_SAVEALL )

#ifndef MOAB_NO_ITAPS_EXTENSIONS

#define iMeshP_getCommunicator ITAPS_FC_WRAPPER( imeshp_getcommunicator, IMESHP_GETCOMMUNICATOR )
#define iMeshP_assignGlobalIds ITAPS_FC_WRAPPER( imeshp_assignglobalids, IMESHP_ASSIGNGLOBALIDS )

#endif

#endif
