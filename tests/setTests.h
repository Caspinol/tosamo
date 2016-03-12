#ifndef _TESTS_H_
#define _TESTS_H_

#include <stdlib.h>
#include <stdio.h>
#include "CuTest.h"

#include "../src/include/settings.h"
#include "../src/include/utils.h"
#include "../src/include/lists.h"


void TestTrim(CuTest *tc);
void TestItos(CuTest *tc);
void TestReverseStr(CuTest *tc);
void TestListCreate(CuTest *tc);
void TestListStuff(CuTest *tc);
void TestListDestroy(CuTest *tc);
void TestSettingsParser(CuTest *tc);
CuSuite* TestUtilsModule();

#endif
