/*=========================================================================

 Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
 Clifton Park, NY, 12065, USA.

 All rights reserved. No part of this software may be reproduced, distributed,
 or modified, in any form or by any means, without permission in writing from
 Kitware Inc.

 IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
 DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
 EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 INCLUDING,
 BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
 "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
 PROVIDE
 MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 =========================================================================*/
// .NAME SystemConfig.h - An .h included by every one for build configuration/macros.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_util_SystemConfig_h
#define __smtk_util_SystemConfig_h

//Windows specific stuff
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#  if !defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#    pragma warning ( disable : 4251 ) /* missing DLL-interface */
#  endif //!defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#endif //Windows specific stuff

#endif //__smtk_util_SystemConfig_h
