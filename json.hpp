#ifndef QUICK_JSON_HPP
#define QUICK_JSON_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <cstddef>

namespace quick {

struct Json {
  enum Type {NULL_TYPE = 0, INT_TYPE = 1, STRING_TYPE = 2,
                      LIST_TYPE = 3, MAP_TYPE = 4, BOOL_TYPE = 5};
  using MapType = std::map<std::string, Json>;
  using ListType = std::vector<Json>;
  template<int type> const auto& Get() const { return std::get<type>(data); }
  template<int type> auto& Get() { return std::get<type>(data); }
  template<int type> bool Has() { return data.index() == type; }
  Type GetType() const { return static_cast<Type>(data.index()); }

  template<int type, typename... Args>
  void Emplace(Args&&... args) { data.emplace(std::forward<Args>(args)...); }

  Json(const Json&) = default;
  Json(Json& x) : Json(std::as_const(x)) { }
  Json(Json&&) = default;
  Json& operator=(const Json&) = default;
  Json& operator=(Json& x) { return operator=(std::as_const(x)); };
  Json& operator=(Json&&) = default;

  template<typename... Args>
  Json(Args&&... args) : data(std::forward<Args>(args)...) { }

  template<typename Arg>
  Json& operator=(Arg&& arg) { data = std::forward<Arg>(arg); return *this; }

  auto& v() { return data; }
  const auto& v() const { return data; }

 private:
  std::variant<std::nullptr_t, int64_t, std::string, ListType, MapType, bool> data;
};

Json ParseJson(const std::string& json_str);
std::string UnparseJson(const Json& json, int indent=2, bool is_inline=false,
                        int depth=0);

}  // namespace quick


#endif // QUICK_JSON_HPP
