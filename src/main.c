#include <Elementary.h>
#include <Ecore.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include "instance_lock.h"
#include "ui.h"
#include "logic.h"

#define _(String) gettext(String)

static void print_usage(void)
{
    printf("Usage: nl-ease [options]\n");
    printf("  (no args)     -> start gui\n");
    printf("  --daemon      -> start daemon mode (background)\n");
    printf("  --help        -> show this help\n");
}

int main(int argc, char **argv)
{
    // Check arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--daemon") == 0) {
            ecore_init();
            // no going back
            logic_run_daemon();        
            ecore_shutdown();         
            return 0;
        }
        else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    // GUI
    if (!lock_gui())
        return 0;
    
    elm_init(argc, argv);

    logic_init();

    setlocale(LC_ALL, "");
    bindtextdomain("nl-ease", "/usr/share/locale");
    textdomain("nl-ease");

    ui_init();

    elm_run();
    elm_shutdown();

    unlock_gui();
    return 0;
}
