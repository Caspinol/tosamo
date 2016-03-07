#include <stdlib.h>
#include <stdio.h>
#include "CuTest.h"
#include "setTests.h"

CuSuite* TestConfigParserModule();

void RunAllTests(void){
  CuString *output = CuStringNew();
  CuSuite* suite = CuSuiteNew();

  CuSuiteAddSuite(suite, TestUtilsModule());

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
}

int main(){
  RunAllTests();

  return 0;
}
