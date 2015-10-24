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

AUTOMAKE_OPTIONS = parallel-tests
TESTS = @PASSLIST@
XFAIL_TESTS = @FAILLIST@
TEST_EXTENSIONS = .mobi .fail
MOBI_LOG_COMPILER = ./test.sh
FAIL_LOG_COMPILER = ./test.sh

clean-local:
	-rm -rf tmp
