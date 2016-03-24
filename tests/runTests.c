#include "setTests.h"

local_settings_t main_settings;

void RunAllTests(void){
	
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	
	CuSuiteAddSuite(suite, TestWholeLot());
        
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}


int main(void){
	RunAllTests();
}
