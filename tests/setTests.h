#ifndef _TESTS_H_
#define _TESTS_H_

#include "CuTest.h"

void TestSubstrDelete(CuTest *tc);
void TestTrim(CuTest *tc);
void TestItos(CuTest *tc);
void TestReverseStr(CuTest *tc);
void TestListCreate(CuTest *tc);
void TestListPush(CuTest *tc);
void TestListDestroy(CuTest *tc);
void TestSettingsParser(CuTest *tc);
CuSuite* TestUtilsModule();

#endif
