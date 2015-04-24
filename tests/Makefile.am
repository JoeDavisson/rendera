# rendera/tests/Makefile.am

PACKAGE_M4 = $(abs_builddir)/package.m4

$(PACKAGE_M4): $(top_srcdir)/configure.ac
	:;{ \
	  echo '# Signature of the current package.' && \
	  echo 'm4_define([AT_PACKAGE_NAME],' && \
	  echo '  [$(PACKAGE_NAME)])' && \
	  echo 'm4_define([AT_PACKAGE_TARNAME],' && \
	  echo '  [$(PACKAGE_TARNAME)])' && \
	  echo 'm4_define([AT_PACKAGE_VERSION],' && \
	  echo '  [$(PACKAGE_VERSION)])' && \
	  echo 'm4_define([AT_PACKAGE_STRING],' && \
	  echo '  [$(PACKAGE_STRING)])' && \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
	  echo '  [$(PACKAGE_BUGREPORT)])'; \
	  echo 'm4_define([AT_PACKAGE_URL],' && \
	  echo '  [$(PACKAGE_URL)])'; \
	} >$@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && \
	  $(SHELL) ./config.status $(subdir) $(subdir)/$@

TESTSUITE = $(builddir)/testsuite
EXTRA_DIST = testsuite.at #$(TESTSUITE) package.m4 testsuite.at.in testsuite 
# CONFIG_CLEAN_FILES = atconfig package.m4 testsuite.log testsuite.at $(TESTSUITE)
CLEANFILES = atconfig testsuite.log testsuite.at # $(TESTSUITE)
# DISTCLEANFILES = testsuite
# DISTCLEANFILES = $(TESTSUITE)
DISTCLEANFILES = $(PACKAGE_M4)

check-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
	  $(TESTSUITEFLAGS)

clean-local: $(TESTSUITE)
	@echo $@
	test ! -f '$(TESTSUITE)' || \
	  $(SHELL) '$(TESTSUITE)' --clean
	rm -vrf $^

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest

$(TESTSUITE): testsuite.at $(PACKAGE_M4)
	$(AUTOTEST) -I '$(abs_srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

