# Parallel tests suite

# Files for testing should be placed in samples directory.
# Normal files must have ".mobi" extension.
# Files that are expected to fail the tests should have ".fail" extension.
# Test script will try to recreate markup sources and dump rawml file.
# Script may additionally check md5 checksums of the produced output.
# In order to enable md5 verification files with checksums must be present in md5 directory.
# Name of the file with md5 checksums is md5 checksum of the sample file plus
# suffix "_rawml" for rawml checksum and "_markup" for all markup files checksums.
# Re-run ./configure after adding new samples

# Exclude large samples from dist package
EXTRA_DIST = md5 \
             samples/sample-cp1252.mobi \
             samples/sample-dict-infl2.mobi \
             samples/sample-drm_pidLTKULBB^5V-v2.mobi \
             samples/sample-drm-v1.mobi \
             samples/sample-multimedia.mobi \
             samples/sample-ncx.mobi \
             samples/sample-obfuscated-fonts.mobi \
             samples/sample-textread.mobi \
             samples/sample-unicode-huffdic.mobi \
             samples/sample-unicode-uncompressed.mobi \
             samples/sample-invalid-indx.fail
AUTOMAKE_OPTIONS = parallel-tests
TESTS = @TESTLIST@
XFAIL_TESTS = @FAILLIST@
TEST_EXTENSIONS = .mobi .fail
MOBI_LOG_COMPILER = ./test.sh
FAIL_LOG_COMPILER = ./test.sh

clean-local:
	-rm -rf tmp

all-local:
	@if test x@RUN_TESTS@ = xno; then \
		echo "============================================================================"; \
		echo "WARNING: All tests will be skipped, because bash was not found in your PATH."; \
		echo "         Please install bash and rerun configure script."; \
		echo "============================================================================"; \
	fi
