//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/RemusBridgeConnection.h"
#include "smtk/bridge/remote/RemusRemoteBridge.h"

#include "smtk/AutoInit.h"

#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/common/UUID.h"

using namespace smtk::model;
using namespace smtk::bridge::remote;

#ifdef SMTK_BUILD_CGM
smtkComponentInitMacro(smtk_cgm_bridge);
#endif

int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    std::cout
      << "Usage:\n"
      << "  " << argv[0] << " filename hint [remusHost [remusPort]]\n"
      << "where\n"
      << "  filename     is the path to a model file to read.\n"
      << "  hint         is the name of an SMTK bridge or worker.\n"
      << "  remusHost    is either \"local\" or a hostname where a\n"
      << "               remus server is running. \"local\" is default.\n"
      << "  remusPort    is the port number of the remus server.\n"
      << "               The default port is 50505.\n"
      ;
    return 1;
    }

  std::string fileName(argv[1]);
  std::string bridgeName(argc > 2 ? argv[2] : "");
  std::string hostname(argc > 3 ? argv[3] : "local");
  int port(argc > 4 ? atoi(argv[4]) : 50505);

  RemusBridgeConnection::Ptr bconn =
    RemusBridgeConnection::create();
  // Do not search for workers in default paths; we don't want to pick things up by accident:
  bconn->clearSearchDirs(true);
  if (argc > 5)
     bconn->addSearchDir(argv[5]);

  bool didConnect = true;
  if (hostname != "local")
    didConnect = bconn->connectToServer(hostname, port);
  else
    didConnect = bconn->connectToServer();
  if (!didConnect)
    {
    std::cerr << "Could not connect to tcp://" << hostname << ":" << port << "\n";
    return 2;
    }

  std::vector<std::string> opnames;
  std::vector<std::string> bnames = bconn->bridgeNames();
  std::vector<std::string>::const_iterator strit;
  std::cout << "Bridge names available:\n";
  for (strit = bnames.begin(); strit != bnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  std::set<std::string> bnameset(bnames.begin(), bnames.end());
  if (bnameset.find(bridgeName) == bnameset.end())
    {
    std::cerr << "Bridge \"" << bridgeName << "\" not available.\n";
    return 4;
    }

  opnames = bconn->operatorNames(bridgeName);
  std::cout << "Operators for bridge \"" << bridgeName << "\":\n";
  for (strit = opnames.begin(); strit != opnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  std::vector<std::string> fileTypes = bconn->supportedFileTypes(bridgeName);
  std::cout << "File types available for \"" << bridgeName << "\":\n";
  for (strit = fileTypes.begin(); strit != fileTypes.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  smtk::common::UUID bridgeSessionId = bconn->beginBridgeSession(bridgeName);
  if (bridgeSessionId.isNull())
    {
    std::cerr << "Null bridge session ID.\n";
    return 8;
    }
  std::cout << "Started session " << bridgeSessionId.toString() << "\n";

  smtk::model::OperatorResult readResult = bconn->readFile(fileName, "", bridgeName);
  std::string strout;
  switch (readResult->findInt("outcome")->value())
    {
  case smtk::model::UNABLE_TO_OPERATE:   strout = "unable to operate"; break;
  case smtk::model::OPERATION_CANCELED:  strout = "canceled"; break;
  case smtk::model::OPERATION_FAILED:    strout = "failed"; break;
  case smtk::model::OPERATION_SUCCEEDED: strout = "succeeded"; break;
  case smtk::model::OUTCOME_UNKNOWN:
  default:
    strout = "unknown"; break;
    }
  std::cout << "Read file? " << strout << " (" << readResult->findInt("outcome")->value() << ")\n";
  std::cout << "Output model is " << readResult->findModelEntity("model")->value() << "\n";

  opnames = bconn->operatorNames(bridgeSessionId);
  std::cout << "Operators for bridge \"" << bridgeSessionId << "\":\n";
  for (strit = opnames.begin(); strit != opnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  bool didEnd = bconn->endBridgeSession(bridgeSessionId);
  std::cout << "Ended session " << bridgeSessionId.toString() << ": " << (didEnd ? "Y" : "N") << "\n";

  return 0;
}
#endif // SHIBOKEN_SKIP
