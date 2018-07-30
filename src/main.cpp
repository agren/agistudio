/*
 *  QT AGI Studio :: Copyright (C) 2000 Helen Zommer
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>

#include <QApplication>
#include <QMainWindow>

#include "menu.h"
#include "game.h"

QApplication *app;
char tmp[MAX_TMP]; //global temporary buffer

extern TStringList InputLines;

static char help[] =
    "QT AGI Studio v1.1.\n\
A Sierra On-Line(tm) adventure game creator and editor.\n\
\n\
Usage: agistudio [switches] \n\
\n\
where [switches] are optionally:\n\
\n\
-dir GAMEDIR    : open an existing game in GAMEDIR\n\
-decode RESNUM  : decode logic resource nr RESNUM to stdout\n\
-compile RESNUM : compile code from stdin to logic resource nr RESNUM\n\
-rebuildvol     : rebuild VOL files\n\
-help           : this message\n\
\n";

//***************************************************
int main(int argc, char **argv)
{
    char *gamedir = NULL;
    int decodeResNum = -1;
    int compileResNum = -1;
    bool rebuildVOLFiles = false;
    bool showGui = true;

    tmp[0] = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i] + 1, "dir"))
                gamedir = argv[i + 1];
            else if (!strcmp(argv[i] + 1, "decode")) {
                decodeResNum = atoi(argv[i + 1]);
                showGui = false;
            } else if (!strcmp(argv[i] + 1, "compile")) {
                compileResNum = atoi(argv[i + 1]);
                showGui = false;
            } else if (!strcmp(argv[i] + 1, "rebuildvol")) {
                rebuildVOLFiles = true;
                showGui = false;
            } else {
                if (strcmp(argv[i] + 1, "help") != 0 && strcmp(argv[i] + 1, "-help") != 0)
                    printf("Unknown parameter.\n\n");
                printf(help);
                exit(-2);
            }
        }
    }

    app = new QApplication(argc, argv);
    menu = new Menu(NULL, NULL);

    game = new Game();
    game->popups_enabled = showGui;

    if (showGui) {
        menu->show();
    }

    if (gamedir) {
        int err = game->open(gamedir);
        if (err) {
            printf("Failed to open game\n");
            return -5;
        } else {
            if (showGui) {
                menu->show_resources();
            }
        }
    }

    if (showGui) {
        return app->exec();
    } else {
        if (decodeResNum != -1) {
            Logic logic;
            int err = logic.decode(decodeResNum);
            if (err) {
                printf("Error decoding logic.%d\n", decodeResNum);
                return -6;
            }
            printf("%s", logic.OutputText.c_str());
        } else if (compileResNum != -1) {
            Logic logic;
            int err;

            InputLines.lfree();
            for (std::string s; std::getline(std::cin, s); ) {
                InputLines.add(s);
            }
            err = logic.compile();
            if (err) {
                printf("Error encoding logic.%d\n", compileResNum);
                printf("%s\n", logic.ErrorList.c_str());
                return -7;
            }
            err = game->AddResource(LOGIC, compileResNum);
            if (err) {
                printf("Could not add resource\n");
                return -8;
            }
            printf("Encoded logic.%d\n", compileResNum);
        } else if (rebuildVOLFiles) {
            printf("Rebuilding VOL files\n");
            int err = game->RebuildVOLfiles();
            if (err) {
                printf("Failed\n");
                return -9;
            }
        }

        return 0;
    }
}
