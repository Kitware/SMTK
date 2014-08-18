#include "smtk/remote/RemusBridgeConnection.h"
#include "smtk/remote/RemusRemoteBridge.h"

#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/util/UUID.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  if (argc < 4)
    return 1;

  std::string fileName(argv[1]);
  std::string bridgeName(argv[2]);
  std::string hostname(argv[3]);
  int port(argc > 4 ? atoi(argv[4]) : 50505);

  RemusBridgeConnection::Ptr bconn =
    RemusBridgeConnection::create();

  bool didConnect = bconn->connectToServer(hostname, port);
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

  smtk::util::UUID bridgeSessionId = bconn->beginBridgeSession(bridgeName);
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
