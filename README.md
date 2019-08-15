Skeleton plugin for wasora
==========================

This tree contains a template for a generic plugin for [wasora](https://bitbucket.org/seamplex/wasora).
This [markdown](http://daringfireball.net/projects/markdown/)-formatted `README` will be the first documentation your potential users will see, so be sure to write a clear description of your plugin here. The `autogen.sh` script will convert this file to a plain-text `README` and to an HTML version using [pandoc](http://johnmacfarlane.net/pandoc/), provided it is installed. These two converted files will be included in the tarball created using `make dist`. The original `README.md`, however, is usually rendered and displayed when showing the source repository tree by hosting services such as [Bitbucket](http://www.bitbucket.org/) and [Github](https://github.com/).

The rest of the file actually describes how to use this skeleton to build an usable wasora plugin. Be sure to replace this text (and the one contained in `AUTHORS`, `NEWS` and `TODO`) with your own contents when distributing your plugin.


The `autogen.sh` script
-----------------------

This script first generates the files that [autoconf](http://www.gnu.org/software/autoconf/) & friends need to generate the `configure` script (i.e. `configure.ac` and `src/Makefile.am`) and then calls `autoreconf`. Note that `libtool` is also needed, but `autoreconf` will fail with a non-clear error message if it not installed. It it complains with a cryptic message, be sure `libtool` is properly installed.

The script reads version information from the distributed control version systems (either `hg` or `bzr` may be used) to generate the header `src/versionvcs.h`. Be sure to read and understand the contents of `autogen.sh`, because if your plugin needs to be linked against special libraries some modifications will be needed.

As this tree generates a plugin for [wasora](https://bitbucket.org/seamplex/wasora), access its source tree is needed. By default, `autogen.sh` tries to find a valid wasora source tree (either an `hg`-cloned or an uncompressed source tarball) in the following relative locations:

 * `../wasora`
 * `../../wasora`
 * `../../../wasora`

A different location can be provided either

 1. by passing the path as an argument of `autogen.sh`

        $ ./autogen.sh $HOME/wasora

 2. by setting the environment variable `WASORA_DIR`

        $ export WASORA_DIR=$HOME/wasora
        $ ./autogen.sh

The wasora source directory `src` is copied into the plugins tree under `src/wasora`. The associated script `autoclean.sh` should remove any automatically-generated files (including `src/wasora`) and revert the tree to a freshly `hg`-cloned status.

After a successful execution of `autogen.sh`, the `configure` script should be ready to be executed:

    $ ./configure
    checking for a BSD-compatible install... /usr/bin/install -c
    checking whether build environment is sane... yes
    [...]
    config.status: executing depfiles commands
    config.status: executing libtool commands
    
    ## ---------------------------------------------------- ##
    ## Configuration summary for skel standalone executable ##
    ## ---------------------------------------------------- ##
      GSL library (required): yes, version 1.16
    
      IDA library (optional): yes, version unknown
        differential-algebraic systems will be solved
    
      Readline library (opt): yes, version 6.3
        run-time debugging-like capabilities will be provided

    Now proceed to compile with 'make'
    
    $ make
    Making all in src
    make[1]: Entering directory '/home/jeremy/codigos/wasora-suite/skel/src'
    depbase=`echo version.lo | sed 's|[^/]*$|.deps/&|;s|\.lo$||'`;\
    [...]
    make[1]: Leaving directory '/home/jeremy/codigos/wasora-suite/skel'
    $

A simple test suite is included, and can be execute with `make test`.

To compile the code with debugging information, the `CFLAGS=-g` option can be passed to either `configure` or `make`, i.e. 

    $ ./configure CFLAGS=-g
    $ make

or

    $ ./configure
    $ make CFLAGS=-g

Any reasonably modern IDE may be able to create projects associated to existing sources that follow the GNU Autoconf standards.

By default, both a shared-object file with the dynamically-loadable plugin (`skel.so`) and a standalone executable of wasora with the plugin statically linked into it (`skel`). To disable the generation of either one, `configure` provides the options `--disable-plugin` or `--disable-standalone`. The default value (both target enabled) can be modified in the `autogen.sh` script.



The `src` directory
-------------------

The source files of the plugin should be located in the `src`. The `autogen.sh` should detect valid C sources and headers and automatically have them included in the generated makefiles. Should you need to write your plugin in another language (why would you want that?), you will have to modify `autogen.sh` so it can find your source (search for "`find").

A wasora plugin is essentially a shared-object file with the following fixed entry points:

    char *plugin_name(void);
    char *plugin_version(void);
    char *plugin_longversion(void);
    char *plugin_usage(void);
    char *plugin_description(void);
    char *plugin_wasorahmd5(void);
    char *plugin_copyright(void);
    int plugin_init_before_parser(void);
    int plugin_parse_line(char *);
    int plugin_init_after_parser(void);
    int plugin_init_before_run(void);
    int plugin_finalize(void);

The provided `src` directory proposed a way to separate these entry points into several files, but any configuration may be valid---including a single file.

### skel.h

Following wasora coding practices, it is a good idea to include all global declarations and definitions in a single header which is include from the individual source files. Note that this file also includes the header `<wasora.h>`, whose search path is defined in the makefile according to either the `WASORA_DIR` environment variable or the copied source tree `src/wasora`. The included `wasora.h` file should be the same used to compile the wasora executable that will dynamically load the plugin. A run-time check is always performed when loading plugins. See below for details.


### version.c

The information about the plugin name, description and copyright should be entered in this file. The functions 

    char *plugin_name(void);
    char *plugin_version(void);
    char *plugin_longversion(void);
    char *plugin_usage(void);
    char *plugin_description(void);
    char *plugin_wasorahmd5(void);
    char *plugin_copyright(void);

should return pointers to static data, because no `free()` is expected to be performed over the returned pointer. To check that the version information is correctly understood by wasora, run first the standalone executable without arguments:

    $ ./skel
    skel 0.4.0 default (2014-08-18 15:08 -0300)
    dummy plugin skeleton for wasora
    $

and then call it with the `-v` or `--version` option:

    $ ./skel -v
    skel 0.3.0 default (2014-08-18 15:08 -0300)
    dummy plugin skeleton for wasora
    
     revision id 9135d714bb7083b472a2428db207ac669d22eb05
     last commit on 2014-08-18 15:08 -0300 (rev 0 2014-08-18 16:34:58)
     compiled on jeremy by tom@linux-gnu x86_64
     with gcc (Debian 4.9.1-4) 4.9.1 using -g

     skel is copyright (c) 2014 jeremy theler
     licensed under GNU GPL version 3 or later.
     skel is free software: you are free to change and redistribute it.
     There is NO WARRANTY, to the extent permitted by law.
    
    
    --------        --------        --------       ------     ----    ---
    wasora 0.4.8 default (2014-08-18 01:22 -0300)
    wasora's an advanced suite for optimization & reactor analysis
    
     revision id ebdf52852bed58f15c43ca642b23eebdde877493
     last commit on 2014-08-18 01:22 -0300 (rev 8)
    
     compiled on 2014-08-18 01:24:15 by jeremy@tom (linux-gnu x86_64)
     with gcc (Debian 4.9.1-4) 4.9.1 using -O2 and linked against
      GNU Scientific Library version 1.16
      GNU Readline version 6.3
      SUNDIALs Library version 2.5.0
     
     
     wasora is copyright (C) 2009-2014 jeremy theler
     licensed under GNU GPL version 3 or later.
     wasora is free software: you are free to change and redistribute it.
     There is NO WARRANTY, to the extent permitted by law.
    
    $


The function `plugin_wasorahmd5` should not be modified:

    const char skelhmd5[] = PLUGIN_HEADERMD5;
    const char *plugin_wasorahmd5(void) {
      return skelhmd5;
    }

The macro `PLUGIN_HEADERMD5` is automatically defined in `version.h` by `configure` to be equal to the MD5 hash of the header file `wasora.h` used to compile the plugin. When wasora loads a shared-object plugin at run-time it verifies that the plugin was compiled using the same `wasora.h` as the wasora executable that is trying to load the plugin using the `plugin_wasorahmd5()` function. The other strings are arbitrary and can contain any valid ASCII string.


### parser.c

Whenever the wasora parser finds a line in the input file that contains a unknown keyword, it passes that line to all the loaded plugins until one can successfully understand the line. Therefore, the entry point `plugin_parse_line(char *)` is called with the line to be parsed as an argument. If one plugin returns with a `WASORA_PARSER_UNHANDLED` status, the line is passed to the following plugin. If no plugin can understand the line, wasora complains about a syntax error in the input. If one plugin returns `WASORA_PARSER_OK` then the line is accepted and the wasora parser moves to the next line in the input file. If the status `WASORA_PARSER_ERROR` is returned, wasora fails with a parsing error.

See also the file `parser.c` in wasora's source tree to see how keywords and arguments are expected to be parser.

### init.c

There are three functions that should be used to initialize the plugin and one to finalize it. They are

plugin_init_before_parser

:   Called before parsing the input file. Can be used for example to define special variables that are expected to be referred to within the input file.

plugin_init_after_parser

:   Called after successfully parsing the input file. Can be used for example to initialize functions or objects with data read from the input file.

plugin_init_before_run
:   Called before starting an outer step of the execution (parametric, fit and optimization steps). Can be used to re-initialize the initial conditions of the problem.



### skel.c

This file contains the example function `skel_instruction_step` that advances a step of a supposedly complex computation. Such function was included in the list of instructions wasora has to execute when the API function `wasora_define_instruction` was called from `parser.c`.


Distributing
------------

To obtain a tarball that can be used to be distributed call `make dist` after successful compilation:

    $ ./autogen.sh
    $ ./configure
    $ make
    $ make check
    $ make dist

A file `skel-0.4.x.tar.gz` where `skel` is replaced by your plugin's name as defined in `autogen.sh` and `x` by the revision number of the version control system.


Licensing
---------

Wasora is distributed under the terms of the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) version 3 or (at your option) any later version. Any plugin it loads (either as a dynamically-loaded shared object or as a statically linked library) should be distributed only under any [GPL-compatible](http://www.gnu.org/licenses/license-list.html) license (see <http://www.gnu.org/licenses/gpl-faq.html#GPLAndPlugins>).



Further information
-------------------

Home page: <http://www.talador.com.ar/jeremy/wasora>  
Mailing list and bug reports: <wasora@talador.com.ar>  


wasora is copyright (C) 2009--2014 jeremy theler  
wasora is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
wasora is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file `COPYING` for copying conditions.  
