#include "setTests.h"

void TestSettingsParser(CuTest *tc){
	to_parse_local_settings("tests/test.settings");
	
	CuAssertStrEquals(tc, "localhost", main_settings.my_ip);
	CuAssertStrEquals(tc, "localhost", main_settings.remote_ip);
	CuAssertStrEquals(tc, "9666", main_settings.port);
	CuAssertStrEquals(tc, "#%%", main_settings.tag);
}

void TestTrim(CuTest *tc){
	char i1[] = " oki doki    ";
	char i2[] = "";
	char i3[] = " ";

	to_str_trim(i1);
	to_str_trim(i2);
	to_str_trim(i3);
	
	CuAssertStrEquals(tc, "oki doki", i1);
	CuAssertStrEquals(tc, "", i2);
	CuAssertStrEquals(tc, "", i3);
}

void TestReverseStr(CuTest *tc){
	char *s1 = "obliteration";
	char *r1 = to_str_reverse(s1, strlen(s1));
	CuAssertStrEquals(tc, "noitaretilbo", r1);
	
	char *s2 = "";
	char *r2 = to_str_reverse(s2, strlen(s2));
	CuAssertStrEquals(tc, "", r2);
	
	char *s3 = "o";
	char *r3 = to_str_reverse(s3, strlen(s3));
	CuAssertStrEquals(tc, "o", r3);
	
	free(r1);
	free(r2);
	free(r3);
}

void TestItos(CuTest *tc){
	char r1[10];
	to_itos(-134, r1);
	char r2[10];
	to_itos(35, r2);
	char r3[10];
	to_itos(45454545, r3);
	char r4[10];
	to_itos(0, r4);
	
	CuAssertStrEquals(tc, "-134", r1);
	CuAssertStrEquals(tc, "35", r2);
	CuAssertStrEquals(tc, "45454545", r3);
	CuAssertStrEquals(tc, "0", r4);
}

void TestSubstrDelete(CuTest *tc){
	char s1[] = "idzie lisek kolo drogi";
	to_str_del_substr(s1, " kolo", strlen(" kolo"));
	CuAssertStrEquals(tc, "idzie lisek drogi", s1);
	
	char s2[] = "idzie kolo lisek kolo drogi";
	to_str_del_substr(s2, "kolo", strlen("kolo"));
	CuAssertStrEquals(tc, "idzie  lisek kolo drogi", s2);
	
	char s3[] = "idzie lisek kolo drogi";
	to_str_del_substr(s3, "wilk", strlen("wilk"));
	CuAssertStrEquals(tc, "idzie lisek kolo drogi", s3);
	
	char s4[] = "idzie lisek kolo drogi";
	to_str_del_substr(s4, "drogi", strlen("drogi"));
	CuAssertStrEquals(tc, "idzie lisek kolo ", s4);
	
	char s5[] = "lisek";
	to_str_del_substr(s5, "lisek", strlen("lisek"));
	CuAssertStrEquals(tc, "", s5);
	
	char s6[] = "lisek";
	to_str_del_substr(s6, "ise", strlen("ise"));
	CuAssertStrEquals(tc, "lk", s6);
}

void TestListStuff(CuTest *tc){

	L_HEAD *head;

	head = to_list_create();
	CuAssertPtrNotNull(tc, head);
	CuAssertIntEquals(tc, 0, head->count);
	
	KV_PAIR p1 = {.key = "face", .value = "weird"};
	KV_PAIR p2 = {.key = "head", .value = "square"};
	
	to_list_push(head, &p1);
	to_list_push(head, &p2);

	/* Check if counter incremented */
	CuAssertIntEquals(tc, 2, head->count);
	
	KV_PAIR *exp1 = to_list_find(head, "face");
	KV_PAIR *exp2 = to_list_find(head, "head");
	
	CuAssertStrEquals(tc, "weird", exp1->value);
	CuAssertStrEquals(tc, "square", exp2->value);

	exp1 = to_list_get(head, "face");
	exp2 = to_list_get(head, "head");

	/* Check if counter decreased */
	CuAssertIntEquals(tc, 0, head->count);

	/* Try to get it again should be NULL */
	exp1 = to_list_get(head, "face");
	CuAssertPtrEquals(tc, exp1, NULL);
}

CuSuite* TestUtilsModule(){
	
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestTrim);
	SUITE_ADD_TEST(suite, TestListStuff);
	SUITE_ADD_TEST(suite, TestSubstrDelete);
	SUITE_ADD_TEST(suite, TestItos);
	SUITE_ADD_TEST(suite, TestReverseStr);
	SUITE_ADD_TEST(suite, TestSettingsParser);

	return suite;
}
