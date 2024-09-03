#include "tclpd.h"
#include <string.h>
#include <unistd.h>

extern int sys_verbose;  /* included in pd, also defined in s_stuff.h */

/* from tcl_class.c: */
//void source_table_remove(const char *object_name);
void source_table_add(const char *object_name, const char *source_path);

extern int tclpd_do_load_lib(t_canvas *canvas, char *objectname) {
    char filename[MAXPDSTRING], dirbuf[MAXPDSTRING], buf[MAXPDSTRING],
        *classname, *nameptr;
    int fd;

    if ((classname = strrchr(objectname, '/')) != NULL)
        classname++;
    else
        classname = objectname;

    if(sys_onloadlist(objectname)) {
        logpost(0, PD_DEBUG, "tclpd loader: already loaded: %s", objectname);
        return 1;
    }

    /* try looking in the path for (objectname).(tcl) ... */
    if(sys_verbose)
        logpost(0, PD_DEBUG, "tclpd loader: searching for %s in path...", objectname);
    if ((fd = canvas_open(canvas, objectname, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto found;

    /* next try (objectname)/(classname).(tcl) ... */
    strncpy(filename, objectname, MAXPDSTRING);
    filename[MAXPDSTRING - 2] = 0;
    strcat(filename, "/");
    strncat(filename, classname, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING - 1] = 0;
    if(sys_verbose)
        logpost(0, PD_DEBUG, "tclpd loader: searching for %s in path...", filename);
    if ((fd = canvas_open(canvas, filename, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto found;

    if(sys_verbose)
        logpost(0, PD_DEBUG, "tclpd loader: found nothing!");
    return 0;

found:
    logpost(0, PD_DEBUG, "tclpd loader: found!");
    close(fd);
    class_set_extern_dir(gensym(dirbuf));
    /* rebuild the absolute pathname */
    strncpy(filename, dirbuf, MAXPDSTRING);
    filename[MAXPDSTRING - 2] = 0;
    strcat(filename, "/");
    strncat(filename, nameptr, MAXPDSTRING - strlen(filename));
    filename[MAXPDSTRING - 1] = 0;
    logpost(0, PD_DEBUG, "tclpd loader: absolute path is %s", filename);

    int result;

    // create the required tcl namespace for the class
    logpost(0, PD_DEBUG, "tclpd loader: init namespace for class %s", classname);
    tclpd_class_namespace_init(classname);

    // add current dir to the Tcl auto_path so objects can use local packages
    Tcl_Eval(tclpd_interp, "set current_auto_path $auto_path");
    snprintf(buf, MAXPDSTRING, "set auto_path \"{%s} $auto_path\"", dirbuf);
    Tcl_Eval(tclpd_interp, buf);
    logpost(0, PD_NORMAL, "%s", buf);

    // load tcl external:
    logpost(0, PD_DEBUG, "tclpd loader: loading tcl file %s", filename);
    result = Tcl_EvalFile(tclpd_interp, filename);
    if(result == TCL_OK) {
        source_table_add(classname, filename);
        logpost(0, PD_NORMAL, "tclpd loader: loaded %s", filename);
    } else {
        pd_error(0, "tclpd loader: error trying to load %s", filename);
        tclpd_interp_error(NULL, result);
        return 0;
    }
    // reset auto_path
    Tcl_Eval(tclpd_interp, "set auto_path $current_auto_path");

#ifdef TCLPD_CALL_SETUP
    // call the setup method:
    char cmd[64];
    snprintf(cmd, 64, "::%s::setup", classname);
    logpost(0, PD_DEBUG, "tclpd loader: calling setup function for %s", classname);
    result = Tcl_Eval(tclpd_interp, cmd);
    if(result == TCL_OK) {
    } else {
        pd_error(0, "tclpd loader: error in %s %s::setup", filename, classname);
        tclpd_interp_error(NULL, result);
        return 0;
    }
#endif // TCLPD_CALL_SETUP

    class_set_extern_dir(&s_);
    sys_putonloadlist(objectname);
    return 1;
}

