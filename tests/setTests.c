#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CuTest.h"

#include "setTests.h"
#include "../src/include/utils.h"
#include "../src/include/lists.h"
#include "../src/include/settings.h"

L_HEAD *node;

void TestSettingsParser(CuTest *tc){
  L_HEAD *settings = to_parse_settings("tests/test.settings");

  CuAssertPtrNotNull(tc, settings);

  CuAssertStrEquals(tc, "value1", to_list_find(settings, "key1"));
  CuAssertStrEquals(tc, "value2", to_list_find(settings, "key2"));
  CuAssertStrEquals(tc, "value 3", to_list_find(settings, "key 3"));
  to_list_destroy(settings);
}

void TestTrim(CuTest *tc){
  char i1[] = " oki doki    ";
  char i2[] = "";
  char i3[] = " ";
  to_str_trim(i1);
  to_str_trim(i2);
  to_str_trim(i3);
  char *exp1 = "oki doki";
  char *exp2 = "";
  CuAssertStrEquals(tc, exp1, i1);
  CuAssertStrEquals(tc, exp2, i2);
  CuAssertStrEquals(tc, exp2, i3);
}

void TestListCreate(CuTest *tc){
  node = to_list_create("filename");
  CuAssertPtrNotNull(tc, node);
  CuAssertStrEquals(tc,"filename", node->filename);
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

void TestListPush(CuTest *tc){
    
  KV_PAIR p1 = {.key = "face", .value = "weird"};
  KV_PAIR p2 = {.key = "head", .value = "square"};
  to_list_push(node, &p1);
  to_list_push(node, &p2);

  char *p2Expected = to_list_find(node, "head");
  char *p1Expected = to_list_find(node, "face");

  CuAssertStrEquals(tc, "square", p2Expected);
  CuAssertStrEquals(tc, "weird", p1Expected);
}

CuSuite* TestUtilsModule(){
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTrim);
  SUITE_ADD_TEST(suite, TestListCreate);
  SUITE_ADD_TEST(suite, TestListPush);
  SUITE_ADD_TEST(suite, TestSubstrDelete);
  SUITE_ADD_TEST(suite, TestItos);
  SUITE_ADD_TEST(suite, TestReverseStr);
  SUITE_ADD_TEST(suite, TestSettingsParser);
  return suite;
}
