#include "json.hpp"
#include "json.cpp"

#include <iostream>

using quick::Json;
using quick::ParseJson;
using std::string;

void Test_JsonInt() {
  Json j = 11;
  string js = UnparseJson(j);
  assert(js == "11");
  auto j2 = ParseJson(js);
  assert(j2.Get<Json::INT_TYPE>() == 11);
  assert(UnparseJson(j2) == "11");
}

void Test_JsonNull() {
  Json j = nullptr;
  string js = UnparseJson(j);
  assert(js == "null");
  auto j2 = ParseJson(js);
  assert(j2.GetType() == Json::NULL_TYPE);
  assert(UnparseJson(j2) == "null");
}

void Test_JsonStr() {
  Json j = "Abc";
  string js = UnparseJson(j);
  assert(js == "\"Abc\"");
  auto j2 = ParseJson(js);
  assert(j2.Get<Json::STRING_TYPE>() == "Abc");
  assert(UnparseJson(j2) == "\"Abc\"");
}

void Test_JsonListEmpty() {
  Json j = Json::ListType {};
  string js = UnparseJson(j);
  assert(js == "[\n]");
  auto j2 = ParseJson(js);
  assert(j2.Get<Json::LIST_TYPE>().size() == 0);
  assert(UnparseJson(j2) == "[\n]");
}

void Test_JsonListSingleElement() {
  Json j = Json::ListType {11};
  string js = UnparseJson(j);
  assert(js == "[\n  11]");
  auto j2 = ParseJson(js);
  assert(j2.Get<Json::LIST_TYPE>().size() == 1);
  assert(j2.Get<Json::LIST_TYPE>()[0].Get<Json::INT_TYPE>() == 11);
  assert(UnparseJson(j2) == "[\n  11]");
}

void Test_JsonListBasic() {
  Json j = Json::ListType {11, "A", false, nullptr, true};
  string js = UnparseJson(j);
  string expected = "[\n  11,\n  \"A\",\n  false,\n  null,\n  true]";
  assert(js == expected);
  auto j2 = ParseJson(js);
  assert(j2.Get<Json::LIST_TYPE>().size() == 5);
  assert(j2.Get<Json::LIST_TYPE>()[0].Get<Json::INT_TYPE>() == 11);
  assert(j2.Get<Json::LIST_TYPE>()[1].Get<Json::STRING_TYPE>() == "A");
  assert(j2.Get<Json::LIST_TYPE>()[2].Get<Json::BOOL_TYPE>() == false);
  assert(j2.Get<Json::LIST_TYPE>()[3].GetType() == Json::NULL_TYPE);
  assert(j2.Get<Json::LIST_TYPE>()[3].Get<Json::NULL_TYPE>() == nullptr);
  assert(j2.Get<Json::LIST_TYPE>()[4].Get<Json::BOOL_TYPE>() == true);
  assert(UnparseJson(j2) == expected);
}

void Test_JsonEmptyMap() {
  Json j = Json::MapType {};
  string js = UnparseJson(j);
  assert(js == "{\n}");
  auto j2 = ParseJson(js);
  assert(j2.GetType() == Json::MAP_TYPE);
  assert(j2.Get<Json::MAP_TYPE>().size() == 0);
  assert(UnparseJson(j2) == "{\n}");
}

void Test_JsonMapOneKey() {
  Json j = Json::MapType {{"A", 23}};
  string js = UnparseJson(j);
  assert(js == "{\n  \"A\": 23}");
  auto j2 = ParseJson(js);
  assert(j2.GetType() == Json::MAP_TYPE);
  assert(j2.Get<Json::MAP_TYPE>().size() == 1);
  assert(j2.Get<Json::MAP_TYPE>()["A"].Get<Json::INT_TYPE>() == 23);
  assert(UnparseJson(j2) == "{\n  \"A\": 23}");
}

void Test_JsonMapBasic() {
  Json ji = 11;
  Json js("mit");
  Json jm = Json::MapType {{"A", ji}, {"B", js}};
  Json jb = false;
  Json jx = Json::ListType {ji, js, jm, jb, false, nullptr};
  Json jp = Json::MapType {{"X", 444}, {"Y", jx}};
  auto str  = UnparseJson(jp);
  Json parsed_json = ParseJson(str);
  auto str2 = UnparseJson(parsed_json);
}

int main() {
  Test_JsonInt();
  Test_JsonNull();
  Test_JsonStr();
  Test_JsonListEmpty();
  Test_JsonListSingleElement();
  Test_JsonListBasic();
  Test_JsonEmptyMap();
  Test_JsonMapOneKey();
  Test_JsonMapBasic();
}
