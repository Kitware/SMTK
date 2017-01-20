#ifndef IREL_LASSO_HPP
#define IREL_LASSO_HPP

#include "iRel.h"

#define RETURN(CODE) ERROR((CODE), "")
#define RETURNR(CODE) ERRORR((CODE), "")

#define ERROR(CODE, MSG)                              \
  do {                                                \
    *err = LASSOI->set_last_error((CODE), (MSG));     \
    return;                                           \
  } while(false)

#define ERRORR(CODE, MSG) return LASSOI->set_last_error((CODE), (MSG))

#define CHK_ERROR(CODE)                               \
  do {                                                \
    *err = (CODE);                                    \
    if (*err != iBase_SUCCESS)                        \
      return;                                         \
  } while(false)

#define CHK_ERRORR(CODE)                              \
  do {                                                \
    int err = (CODE);                                 \
    if (err != iBase_SUCCESS)                         \
      return err;                                     \
  } while(false)

#define CHK_PAIR()                                    \
  do {                                                \
    if (NULL == pair) {                               \
      ERROR(iBase_FAILURE, "Invalid relation pair."); \
    }                                                 \
  } while(false)

#endif
