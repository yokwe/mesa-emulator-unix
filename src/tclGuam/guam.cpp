 /*
  * hello.c -- A minimal Tcl C extension.
  */
 #include <tcl.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

 static int 
 Hello_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
 {
    (void)cdata;
    (void)objc;
    (void)objv;
     Tcl_SetObjResult(interp, Tcl_NewStringObj("Hello", -1));
     logger.info("hello");
     return TCL_OK;
 }

 static int 
 Boot_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
 {
    (void)cdata;
    (void)objc;
    (void)objv;
     Tcl_SetObjResult(interp, Tcl_NewStringObj("Boot", -1));
     logger.info("boot");
     return TCL_OK;
 }

 /*
  * Hello_Init -- Called when Tcl loads your extension.
  */
extern "C" int DLLEXPORT
 Guam_Init(Tcl_Interp *interp)
 {
     if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
         return TCL_ERROR;
     }
     /* changed this to check for an error - GPS */
     if (Tcl_PkgProvide(interp, "Guam", "1.0") == TCL_ERROR) {
         return TCL_ERROR;
     }
     Tcl_CreateObjCommand(interp, "guam::hello", Hello_Cmd, NULL, NULL);
     Tcl_CreateObjCommand(interp, "guam::boot", Boot_Cmd, NULL, NULL);
     return TCL_OK;
 }
