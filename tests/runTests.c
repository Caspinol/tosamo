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

local_settings_t main_settings;

int main(void){
	RunAllTests();
}
