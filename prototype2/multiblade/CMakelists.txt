

#
# Multiblade Tests
#
set(MBDummyTest_SRC multiblade/mbcaen/DummyClassTest.cpp multiblade/mbcaen/DummyClass.cpp)
set(MBDummyTest_INC test/TestBase.h multiblade/mbcaen/DummyClass.h)
create_test_executable(MBDummyTest "")
