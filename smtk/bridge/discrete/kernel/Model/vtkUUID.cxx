//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

//=============================================================================
//
//  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
//  l'Image). All rights reserved. See Doc/License.txt or
//  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
//
//     This software is distributed WITHOUT ANY WARRANTY; without even
//     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//     PURPOSE.  See the above copyright notices for more information.
//
//=============================================================================
#include "vtkUUID.h"

#include "VTKUUIDConfigure.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>

#if defined(_WIN32) || defined(__CYGWIN__)
#define HAVE_UUIDCREATE
#include <rpc.h>
#elif defined(SYSTEM_UUID_FOUND)
#define HAVE_UUID_GENERATE
#include "uuid/uuid.h"
#endif

// For GetMACAddress
#ifdef _WIN32
#include <conio.h>
#include <snmp.h>
#else
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef CMAKE_HAVE_SYS_IOCTL_H
#include <sys/ioctl.h> // For SIOCGIFCONF on Linux
#endif
#ifdef CMAKE_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef CMAKE_HAVE_SYS_SOCKIO_H
#include <sys/sockio.h> // For SIOCGIFCONF on SunOS
#endif
#ifdef CMAKE_HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef CMAKE_HAVE_NETINET_IN_H
#include <netinet/in.h> //For IPPROTO_IP
#endif
#ifdef CMAKE_HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#if defined(CMAKE_HAVE_NET_IF_ARP_H) && defined(__sun)
// This is absolutely necessary on SunOS
#include <net/if_arp.h>
#endif

vtkStandardNewMacro(vtkUUID);

void vtkUUID::ConstructUUID(unsigned char uuid[16])
{
  // set the random seed the 1st time
  static bool firstTime = true;
  if (firstTime)
  {
    vtkMath::RandomSeed(static_cast<long>(vtksys::SystemTools::GetTime()));
    firstTime = false;
  }

  size_t offset = 0;

  unsigned char macAddress[6];
  // if successful, set 1st 3 bytes of the uuid to be the last 3 bytes of the
  // MAC address.  Not using the 1st 3 bytes because I've read concern about
  // having uuid that identifies the machine it was created on.  At least by
  // eliminating the 1st 3 bytes, the device manufacturer is now hidden
  if (vtkUUID::GetMACAddress(macAddress) != -1)
  {
    uuid[0] = macAddress[3];
    uuid[1] = macAddress[4];
    uuid[2] = macAddress[5];
    offset = 3;
  }

  // up to 3 bytes from the hostname (would like something other than just to
  // rely on the random number generator)... for grins, let's take it from the
  // end of the hostname where, if there are several machines of similar name
  // at a site, there is likely (?) to be a number at the end
  char hostName[256];
  if (gethostname(hostName, sizeof(hostName)) != -1)
  {
    hostName[sizeof(hostName) - 1] = '\0'; // just to be sure
    int bytesAdded = 0, strOffset = static_cast<int>(strlen(hostName)) - 1;
    int msb, lsb;
    while (bytesAdded++ < 3 && strOffset > 0)
    {
      // not perfect (not all equally likely), but at least all possible
      msb = hostName[strOffset--] % 16;
      lsb = hostName[strOffset--] % 16;
      uuid[offset++] = static_cast<unsigned char>((msb << 4) + lsb);
    }
  }

  // generate remaining bytes for the uuid from random # generator
  while (offset < 16)
  {
    double randomVal = vtkMath::Random();
    size_t bytesToCopy = sizeof(double) >> 1; // only use have the random #
    if (bytesToCopy > 16 - offset)
    {
      bytesToCopy = 16 - offset;
    }
    memcpy(uuid + offset, &randomVal, bytesToCopy);
    offset += bytesToCopy;
  }
}

void vtkUUID::ConvertBinaryUUIDToString(unsigned char* uuid, std::string& uuidString)
{
  char buffer[8]; // why more than 3?
  uuidString.clear();
  for (int i = 0; i < 16; i++)
  {
    sprintf(buffer, "%.2x", uuid[i]);
    uuidString += buffer;

    if (i == 3 || i == 5 || i == 7 || i == 9)
    {
      uuidString += "-";
    }
  }
}

int vtkUUID::GenerateUUID(unsigned char uuid[16])
{
#ifdef HAVE_UUIDCREATE
  if (FAILED(UuidCreate(reinterpret_cast<GUID*>(uuid))))
  {
    return -1;
  }
#elif defined(HAVE_UUID_GENERATE)
  uuid_t g;
  uuid_generate(g);
  memcpy(uuid, g, sizeof(uuid_t));
#else
  uuid[0] = 0; // so won't complain about being unused
  return -1;
#endif

  return 0;
}

void vtkUUID::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkUUID::GetMACAddress(unsigned char addr[6])
{
  // This code is the result of a long internet search to find something
  // as compact as possible (not OS independant). We only have to separate
  // 3 OS: Win32, SunOS and 'real' POSIX
  // http://groups-beta.google.com/group/comp.unix.solaris/msg/ad36929d783d63be
  // http://bdn.borland.com/article/0,1410,26040,00.html

  int stat = vtkUUID::GetMacAddrSys(addr);
  if (stat != 0)
  {
    vtkGenericWarningMacro("Problem in finding the MAC Address");
    return -1;
  }

  return 0;
}

#ifdef __sgi
static int SGIGetMacAddress(unsigned char* addr)
{
  FILE* f = popen("/etc/nvram eaddr", "r");
  if (f == 0)
  {
    return -1;
  }
  unsigned int x[6];
  if (fscanf(f, "%02x:%02x:%02x:%02x:%02x:%02x", x, x + 1, x + 2, x + 3, x + 4, x + 5) != 6)
  {
    pclose(f);
    return -1;
  }
  for (unsigned int i = 0; i < 6; i++)
  {
    addr[i] = static_cast<unsigned char>(x[i]);
  }
  return 0;
}
#endif

#ifdef _WIN32
typedef BOOL(WINAPI* pSnmpExtensionInit)(IN DWORD dwTimeZeroReference,
  OUT HANDLE* hPollForTrapEvent, OUT AsnObjectIdentifier* supportedView);

typedef BOOL(WINAPI* pSnmpExtensionTrap)(OUT AsnObjectIdentifier* enterprise,
  OUT AsnInteger* genericTrap, OUT AsnInteger* specificTrap, OUT AsnTimeticks* timeStamp,
  OUT RFC1157VarBindList* variableBindings);

typedef BOOL(WINAPI* pSnmpExtensionQuery)(IN BYTE requestType,
  IN OUT RFC1157VarBindList* variableBindings, OUT AsnInteger* errorStatus,
  OUT AsnInteger* errorIndex);

typedef BOOL(WINAPI* pSnmpExtensionInitEx)(OUT AsnObjectIdentifier* supportedView);
#endif

int vtkUUID::GetMacAddrSys(unsigned char* addr)
{
#ifdef _WIN32
  WSADATA WinsockData;
  if ((WSAStartup(MAKEWORD(2, 0), &WinsockData)) != 0)
  {
    std::cerr << "in Get MAC Adress (internal) : This program requires Winsock 2.x!" << std::endl;
    return -1;
  }

  HANDLE PollForTrapEvent;
  AsnObjectIdentifier SupportedView;
  UINT OID_ifEntryType[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 3 };
  UINT OID_ifEntryNum[] = { 1, 3, 6, 1, 2, 1, 2, 1 };
  UINT OID_ipMACEntAddr[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 6 };
  AsnObjectIdentifier MIB_ifMACEntAddr = { sizeof(OID_ipMACEntAddr) / sizeof(UINT),
    OID_ipMACEntAddr };
  AsnObjectIdentifier MIB_ifEntryType = { sizeof(OID_ifEntryType) / sizeof(UINT), OID_ifEntryType };
  AsnObjectIdentifier MIB_ifEntryNum = { sizeof(OID_ifEntryNum) / sizeof(UINT), OID_ifEntryNum };
  RFC1157VarBindList varBindList;
  RFC1157VarBind varBind[2];
  AsnInteger errorStatus;
  AsnInteger errorIndex;
  AsnObjectIdentifier MIB_NULL = { 0, 0 };
  int ret;
  int dtmp;
  int j = 0;

  // Load the SNMP dll and get the addresses of the functions necessary
  HINSTANCE m_hInst = LoadLibrary("inetmib1.dll");
  if (m_hInst < reinterpret_cast<HINSTANCE>(HINSTANCE_ERROR))
  {
    return -1;
  }
  pSnmpExtensionInit m_Init =
    reinterpret_cast<pSnmpExtensionInit>(GetProcAddress(m_hInst, "SnmpExtensionInit"));
  pSnmpExtensionQuery m_Query =
    reinterpret_cast<pSnmpExtensionQuery>(GetProcAddress(m_hInst, "SnmpExtensionQuery"));
  m_Init(GetTickCount(), &PollForTrapEvent, &SupportedView);

  /* Initialize the variable list to be retrieved by m_Query */
  varBindList.list = varBind;
  varBind[0].name = MIB_NULL;
  varBind[1].name = MIB_NULL;

  // Copy in the OID to find the number of entries in the
  // Inteface table
  varBindList.len = 1; // Only retrieving one item
  SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryNum);
  m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
  //   printf("# of adapters in this system : %i\n",
  //          varBind[0].value.asnValue.number);
  varBindList.len = 2;

  // Copy in the OID of ifType, the type of interface
  SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryType);

  // Copy in the OID of ifPhysAddress, the address
  SNMP_oidcpy(&varBind[1].name, &MIB_ifMACEntAddr);

  do
  {
    // Submit the query.  Responses will be loaded into varBindList.
    // We can expect this call to succeed a # of times corresponding
    // to the # of adapters reported to be in the system
    ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
    if (!ret)
    {
      ret = 1;
    }
    else
    {
      // Confirm that the proper type has been returned
      ret = SNMP_oidncmp(&varBind[0].name, &MIB_ifEntryType, MIB_ifEntryType.idLength);
    }
    if (!ret)
    {
      j++;
      dtmp = varBind[0].value.asnValue.number;

      // Type 6 describes ethernet interfaces
      if (dtmp == 6)
      {
        // Confirm that we have an address here
        ret = SNMP_oidncmp(&varBind[1].name, &MIB_ifMACEntAddr, MIB_ifMACEntAddr.idLength);
        if (!ret && varBind[1].value.asnValue.address.stream != NULL)
        {
          if ((varBind[1].value.asnValue.address.stream[0] == 0x44) &&
            (varBind[1].value.asnValue.address.stream[1] == 0x45) &&
            (varBind[1].value.asnValue.address.stream[2] == 0x53) &&
            (varBind[1].value.asnValue.address.stream[3] == 0x54) &&
            (varBind[1].value.asnValue.address.stream[4] == 0x00))
          {
            // Ignore all dial-up networking adapters
            std::cerr << "in Get MAC Adress (internal) : Interface #" << j << " is a DUN adapter\n";
            continue;
          }
          if ((varBind[1].value.asnValue.address.stream[0] == 0x00) &&
            (varBind[1].value.asnValue.address.stream[1] == 0x00) &&
            (varBind[1].value.asnValue.address.stream[2] == 0x00) &&
            (varBind[1].value.asnValue.address.stream[3] == 0x00) &&
            (varBind[1].value.asnValue.address.stream[4] == 0x00) &&
            (varBind[1].value.asnValue.address.stream[5] == 0x00))
          {
            // Ignore NULL addresses returned by other network
            // interfaces
            std::cerr << "in Get MAC Adress (internal) :  Interface #" << j
                      << " is a NULL address\n";
            continue;
          }
          memcpy(addr, varBind[1].value.asnValue.address.stream, 6);
        }
      }
    }
  } while (!ret);

  // Free the bindings
  SNMP_FreeVarBind(&varBind[0]);
  SNMP_FreeVarBind(&varBind[1]);
  return 0;
#endif //Win32 version

#ifdef __sgi
  return SGIGetMacAddress(addr);
#endif // __sgi

// implementation for POSIX system
#if defined(CMAKE_HAVE_NET_IF_ARP_H) && defined(__sun)
  //The POSIX version is broken anyway on Solaris, plus would require full
  //root power
  struct arpreq parpreq;
  struct sockaddr_in* psa;
  struct hostent* phost;
  char hostname[MAXHOSTNAMELEN];
  char** paddrs;
  int sock, status = 0;

  if (gethostname(hostname, MAXHOSTNAMELEN) != 0)
  {
    perror("in Get MAC Adress (internal) : gethostname");
    return -1;
  }
  phost = gethostbyname(hostname);
  paddrs = phost->h_addr_list;

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1)
  {
    perror("in Get MAC Adress (internal) : sock");
    return -1;
  }
  memset(&parpreq, 0, sizeof(struct arpreq));
  psa = static_cast<struct sockaddr_in*>(&parpreq.arp_pa);

  memset(psa, 0, sizeof(struct sockaddr_in));
  psa->sin_family = AF_INET;
  memcpy(&psa->sin_addr, *paddrs, sizeof(struct in_addr));

  status = ioctl(sock, SIOCGARP, &parpreq);
  if (status == -1)
  {
    perror("in Get MAC Adress (internal) : SIOCGARP");
    return -1;
  }
  memcpy(addr, parpreq.arp_ha.sa_data, 6);

  return 0;
#elif !defined(_WIN32)
#ifdef CMAKE_HAVE_NET_IF_H
  int sd;
  struct ifreq ifr, *ifrp;
  struct ifconf ifc;
  char buf[1024];
  int n, i;
  unsigned char* a;
#if defined(AF_LINK) && (!defined(SIOCGIFHWADDR) && !defined(SIOCGENADDR))
  struct sockaddr_dl* sdlp;
#endif

//
// BSD 4.4 defines the size of an ifreq to be
// max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
// However, under earlier systems, sa_len isn't present, so the size is
// just sizeof(struct ifreq)
// We should investigate the use of SIZEOF_ADDR_IFREQ
//
#ifdef HAVE_SA_LEN
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define ifreq_size(i) max(sizeof(struct ifreq), sizeof((i).ifr_name) + (i).ifr_addr.sa_len)
#else
#define ifreq_size(i) sizeof(struct ifreq)
#endif // HAVE_SA_LEN

  if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
  {
    return -1;
  }
  memset(buf, 0, sizeof(buf));
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sd, SIOCGIFCONF, reinterpret_cast<char*>(&ifc)) < 0)
  {
    close(sd);
    return -1;
  }
  n = ifc.ifc_len;
  for (i = 0; i < n; i += ifreq_size(*ifrp))
  {
    ifrp = reinterpret_cast<struct ifreq*>(static_cast<char*>(ifc.ifc_buf) + i);
    strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
      continue;
    }
    a = reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data);
#else
#ifdef SIOCGENADDR
    // In theory this call should also work on Sun Solaris, but apparently
    // SIOCGENADDR is not implemented properly thus the call
    // ioctl(sd, SIOCGENADDR, &ifr) always returns errno=2
    // (No such file or directory)
    // Furthermore the DLAPI seems to require full root access
    if (ioctl(sd, SIOCGENADDR, &ifr) < 0)
    {
      continue;
    }
    a = static_cast<(unsigned char *>(ifr.ifr_enaddr);
#else
#ifdef AF_LINK
    sdlp = reinterpret_cast<struct sockaddr_dl*>(&ifrp->ifr_addr);
    if ((sdlp->sdl_family != AF_LINK) || (sdlp->sdl_alen != 6))
      continue;
    a = reinterpret_cast<unsigned char*>(&sdlp->sdl_data[sdlp->sdl_nlen]);
#else
    perror("in Get MAC Adress (internal) : No way to access hardware");
    close(sd);
    return -1;
#endif // AF_LINK
#endif // SIOCGENADDR
#endif // SIOCGIFHWADDR
    if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5])
      continue;

    if (addr)
    {
      memcpy(addr, a, 6);
      close(sd);
      return 0;
    }
  }
  close(sd);
#endif
  // Not implemented platforms (or no cable !)
  perror("in Get MAC Adress (internal) : There was a configuration problem (or no cable !) on your "
         "platform");
  memset(addr, 0, 6);
  return -1;
#endif //__sun
}
