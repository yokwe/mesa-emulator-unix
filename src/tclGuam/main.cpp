//
//
//

#include <filesystem>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include <tcl.h>
// #include <tk.h>

#include "guam.h"

int AppInit(Tcl_Interp *interp) {
	if (Tcl_Init(interp) == TCL_ERROR) {
        logger.fatal("Tcl_Init failed");
		return TCL_ERROR;
    }
    // if(Tk_Init(interp) == TCL_ERROR) {
    //     logger.fatal("Tk_Init failed");
    //     return TCL_ERROR;
    // }

	// Tcl_SetVar(interp, "tcl_rcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);

    Guam_Init(interp);

    auto guamScriptFile = std::filesystem::path(BUILD_DIR) / "run" / "guam.tcl";
    if (std::filesystem::exists(guamScriptFile)) {
        logger.info("eval  guam scrip  %s", guamScriptFile.c_str());
        auto guamScript = readFile(guamScriptFile);
        Tcl_Eval(interp, guamScript.c_str());
    }

	return TCL_OK;
}
int main(int argc, char *argv[]) {
    Tcl_FindExecutable(argv[0]);
	Tcl_Main(argc, argv, AppInit);
	return 0;
}
