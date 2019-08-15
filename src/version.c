/*------------ -------------- -------- --- ----- ---   --       -            -
 *  mate's version banner
 *
 *  Copyright (C) 2017 ramiro vignolo
 *
 *  This file is part of mate.
 *
 *  mate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  mate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wasora.  If not, see <http://www.gnu.org/licenses/>.
 *------------------- ------------  ----    --------  --     -       -         -
 */

#include <stdio.h>

#include "mate.h"
#include "version.h"

// global static so the compiler locates these strings in the text section
// so when the plugin_* functions return pointers to the strings there is
// no need to free them afterward
const char matename[] = "mate";
const char matedescription[] = "steady-state analysis for systems involving mixtures of steam and water";
char mateshortversion[128];
char matelongversion[2048];
const char mateusage[] = "no commandline options needed";

const char matecopyright[] = "\
 mate is copyright (C) 2017 ramiro vignolo\n\
 licensed under GNU GPL version 3 or later.\n\
 mate is free software: you are free to change and redistribute it.\n\
 There is NO WARRANTY, to the extent permitted by law.";

const char matehmd5[] = PLUGIN_HEADERMD5;


const char *plugin_name(void) {
  return matename;
}

const char *plugin_longversion(void) {
  
#ifdef PLUGIN_VCS_BRANCH
  sprintf(matelongversion, "\n\
 last commit on %s\n\
 compiled on %s by %s@%s (%s)\n\
 with %s using %s\n",
   PLUGIN_VCS_DATE,
   COMPILATION_DATE,
   COMPILATION_USERNAME,
   COMPILATION_HOSTNAME,
   COMPILATION_ARCH,
   CCOMPILER_VERSION,
   CCOMPILER_FLAGS);
#endif

  return matelongversion;
}

const char *plugin_wasorahmd5(void) {
  return matehmd5;
}
const char *plugin_copyright(void) {
  return matecopyright;
}


const char *plugin_version(void) {
#ifdef PLUGIN_VCS_BRANCH
  sprintf(mateshortversion, "%s%s %s", PLUGIN_VCS_VERSION,
                                           (PLUGIN_VCS_CLEAN==0)?"":"+Δ",
                                           strcmp(PLUGIN_VCS_BRANCH, "master")?PLUGIN_VCS_BRANCH:"");
#else
  sprintf(mateshortversion, "%s", PACKAGE_VERSION);
#endif
  
  return mateshortversion;
}

const char *plugin_description(void) {
  return matedescription;
}

const char *plugin_usage(void) {
  return mateusage;
}
