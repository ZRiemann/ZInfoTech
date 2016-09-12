#ifndef _Z_EXPORT_H_
#define _Z_EXPORT_H_

#ifndef ZEXPORT
#define ZEXPORT 1
#endif// ZEXPORT

extern int ZOK; // = ZEOK;// 0 // ZError OK
extern int ZFAIL;// = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
extern int ZMEM_INSUFFICIENT;// = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
extern int ZFUN_FAIL;// = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
extern int ZNOT_SUPPORT;// = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
extern int ZPARAM_INVALID;// = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
extern int ZAGAIN;// = ZEAGAIN;// (ZEMASK |6) // try again
extern int ZTIMEOUT;// = ZETIMEOUT;// (ZEMASK | 7) // operation time out
extern int ZNOT_EXIST;// = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
extern int ZNOT_INIT;// = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
extern int ZSTATUS_INVALID;// = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
extern int ZMEM_OUTOFBOUNDS;// = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
extern int ZCMD_STOP;// = ZECMD_STOP;// (ZEMASK | 12) // return stop command


#endif//_Z_EXPORT_H_
