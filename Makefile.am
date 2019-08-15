include plugin.mk

SUBDIRS = src
DIST_SUBDIRS = src wasora

ACLOCAL_AMFLAGS = -I wasora/m4
dist_doc_DATA = AUTHORS ChangeLog README TODO README doc/${plugin}.texi
EXTRA_DIST = examples doc src/version.sh

TESTS = examples/test.sh

all-local:
	if [ -e src/.libs/lib${plugin}.so.0.0.0 ]; then cp -u  src/.libs/lib${plugin}.so.0.0.0 ./${plugin}.so; fi
	if [ -e src/${plugin} ]; then cp -u  src/${plugin} .; fi
	if [ -e ${plugin} ]; then cd examples; ln -sf ../${plugin}; cd;  fi
	rm -f src/version.$(OBJEXT) src/version.h src/version.lo src/.lib/version.$(OBJEXT)

clean-local:
	rm -f src/wasora/${plugin} examples/${plugin}
	rm -f ${plugin} ${plugin}.so
	rm -f *~ examples/*~
	rm -f examples/*.pos examples/*.vtk
	rm -f *.log *.trs

check-local:
	@echo
	@echo If you want to manually repeat the test, type
	@echo
	@echo ./${plugin} examples/test.was
	@echo


install-exec-local:
