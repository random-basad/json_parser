#include "json.hpp"

#include <sstream>
#include <iostream>
#include <iomanip>

namespace quick {

using std::string;
using std::vector;

static std::string EscapeString(const std::string& input, bool quote=true) {
  std::ostringstream o;
  if (quote) {
    o << '"';
  }
  for (auto c = input.cbegin(); c != input.cend(); ++c) {
    if (*c == '"' || *c == '\\') {
      o << "\\" << *c;
    } else if (*c == '\t') {
      o << "\\t";
    } else if (*c == '\n') {
      o << "\\n";
    } else if ('\x00' <= *c && *c <= '\x1f') {
        o << "\\u"
          << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
    } else {
        o << *c;
    }
  }
  if (quote) {
    o << '"';
  }
  return o.str();
}

static int64_t StrToInt(const string& s) {
  std::stringstream ss(s);
  int64_t output = 0;
  ss >> output;
  return output;
}

static void UnparseJson(const Json& json, std::ostringstream& json_str_stream,
                        int indent, bool is_inline, int depth) {
  switch (json.GetType()) {
    case Json::NULL_TYPE: {
      json_str_stream << "null";
      break;
    }
    case Json::BOOL_TYPE: {
      json_str_stream << (json.Get<Json::BOOL_TYPE>() ? "true": "false");
      break;
    }
    case Json::INT_TYPE: {
      json_str_stream << json.Get<Json::INT_TYPE>();
      break;
    }
    case Json::STRING_TYPE: {
      json_str_stream << EscapeString(json.Get<Json::STRING_TYPE>());
      break;
    }
    case Json::LIST_TYPE: {
      json_str_stream << "[";
      if (not is_inline) { json_str_stream << "\n"; }
      bool is_first_time = true;
      for (auto& c : json.Get<Json::LIST_TYPE>()) {
        if (not is_first_time) {
          json_str_stream << "," << (is_inline ? " " : "\n");
        }
        if (not is_inline) {
          json_str_stream << std::string(depth + indent, ' ');
        }
        UnparseJson(c, json_str_stream, indent, is_inline, depth + indent);
        is_first_time = false;
      }
      json_str_stream << "]";
      break;
    }
    case Json::MAP_TYPE: {
      json_str_stream << "{";
      if (not is_inline) { json_str_stream << "\n"; }
      bool is_first_time = true;
      for (auto& c : json.Get<Json::MAP_TYPE>()) {
        if (not is_first_time) {
          json_str_stream << "," << (is_inline ? " " : "\n");
        }
        if (not is_inline) {
          json_str_stream << std::string(depth + indent, ' ');
        }
        json_str_stream << EscapeString(c.first) << ": ";
        UnparseJson(c.second, json_str_stream, indent, is_inline, depth + indent);
        is_first_time = false;
      }
      json_str_stream << "}";
      break;
    }
  }
}

std::string UnparseJson(const Json& json, int indent, bool is_inline,
                        int depth) {
  std::ostringstream oss;
  UnparseJson(json, oss, indent, is_inline, depth);
  return oss.str();
}

class JsonParser {
 public:
  JsonParser(const std::string& str): input(str), len(str.size()) { }
  enum TokenType {
    NULL_TYPE, STRING_TYPE, INT_TYPE, TRUE_BOOL, FALSE_BOOL,
    COMMA, LIST_BRACE_START, LIST_BRACE_END, MAP_BRACE_START, MAP_BRACE_END,
    COLON, DOUBLE_TYPE
  };
  struct Token {
    TokenType type;
    std::string value;
  };
  enum LexerState {BEGIN, INSIDE_INT, INSIDE_STR};

  Json Parse() const {
    int token_offset = 0;
    assert(tokens.size() > 0);
    return Parse(&token_offset);
  }
  vector<Token>& Tokenize();

 private:

  Json Parse(int* token_offset) const;

  void Tokenize_HandleBegin(int i, char c);

  const std::string& input;
  std::vector<Token> tokens;
  LexerState state = BEGIN;
  string buffer;
  bool is_double = false;
  int len;
  int tokens_len = 0;
};

void JsonParser::Tokenize_HandleBegin(int i, char c) {
  switch (c) {
    case ' ':
    case '\t':
    case '\n': {
      return;
    }
    case '{': {
      tokens.push_back({TokenType::MAP_BRACE_START});
      return;
    }
    case '}': {
      tokens.push_back({TokenType::MAP_BRACE_END});
      return;
    }
    case '[': {
      tokens.push_back({TokenType::LIST_BRACE_START});
      return;
    }
    case ']': {
      tokens.push_back({TokenType::LIST_BRACE_END});
      return;
    }
    case ':': {
      tokens.push_back({TokenType::COLON});
      return;
    }
    case ',': {
      tokens.push_back({TokenType::COMMA});
      return;
    }
    case 't': {
      assert(i+3 < len && input.substr(i, 4) == "true");
      tokens.push_back({TokenType::TRUE_BOOL});
      return;
    }
    case 'f': {
      assert(i+4 < len && input.substr(i, 5) == "false");
      tokens.push_back({TokenType::FALSE_BOOL});
      return;
    }
    case 'n': {
      assert(i+3 < len && input.substr(i, 4) == "null");
      tokens.push_back({TokenType::NULL_TYPE});
      return;
    }
    case '"': {
      state = INSIDE_STR;
      return;
    }
    default: break;
  }
  if (('0' <= c && c <= '9') || c == '-' || c == '+') {
    buffer += c;
    state = INSIDE_INT;
  }
}

vector<JsonParser::Token>& JsonParser::Tokenize() {
  for (int i = 0; i < len; ++i) {
    char c = input[i];
    switch (state) {
      case BEGIN: {
        Tokenize_HandleBegin(i, c);
        break;
      }
      case INSIDE_INT: {
        if (('0' <= c && c <= '9') || c == '.') {
          buffer += c;
          is_double |= (c == '.');
        } else {
          state = BEGIN;
          i--;
          if (not is_double) {
            tokens.push_back({TokenType::INT_TYPE, buffer});
          } else {
            tokens.push_back({TokenType::DOUBLE_TYPE, buffer});
          }
          buffer.clear();
          is_double = false;
        }
        break;
      }
      case INSIDE_STR: {
        if (c == '"') {
          state = BEGIN;
          tokens.push_back({TokenType::STRING_TYPE, buffer});
          buffer.clear();
        } else if (c == '\\') {
          assert(i + 1 < len);
          char nc = input[i];
          switch (nc) {
            case '"':
            case '\\': {
              buffer += nc; break;
            }
            case 'n':
              buffer += '\n'; break;
            case 't':
              buffer += '\t'; break;
            default: assert(false);
          }
          i++;
        } else {
          buffer += c;
        }
        break;
      }
      default: assert(false);
    }
  }
  switch (state) {
    case INSIDE_INT: {
      if (not is_double) {
        tokens.push_back({TokenType::INT_TYPE, buffer});
      } else {
        tokens.push_back({TokenType::DOUBLE_TYPE, buffer});
      }
      break;
    }
    case INSIDE_STR: assert(false);
    default: break;
  }
  tokens_len = tokens.size();
  return tokens;
}

Json JsonParser::Parse(int* token_offset) const {
  auto& first_token = tokens.at(*token_offset);
  switch (first_token.type) {
    case TokenType::NULL_TYPE: {
      *token_offset += 1;
      return Json(nullptr);
    }
    case TokenType::STRING_TYPE: {
      *token_offset += 1;
      return Json(std::move(first_token.value));
    }
    case TokenType::INT_TYPE: {
      *token_offset += 1;
      return Json(StrToInt(first_token.value));
    }
    case TokenType::TRUE_BOOL: {
      *token_offset += 1;
      return Json(true);
    }
    case TokenType::FALSE_BOOL: {
      *token_offset += 1;
      return Json(false);
    }
    case TokenType::LIST_BRACE_START: {
      Json::ListType jlist;
      *token_offset += 1;
      // std::cout << "LS: " << *token_offset << std::endl;
      assert(*token_offset < tokens_len);
      while (tokens.at(*token_offset).type != TokenType::LIST_BRACE_END) {
        jlist.emplace_back(Parse(token_offset));
        assert(*token_offset < tokens_len);
        if (tokens.at(*token_offset).type == TokenType::COMMA) {
          *token_offset += 1;
          assert(*token_offset < tokens_len);
        }
      }
      *token_offset += 1;
      return jlist;
    }
    case TokenType::MAP_BRACE_START: {
      Json::MapType jmap;
      *token_offset += 1;
      assert(*token_offset < tokens_len);
      while (tokens.at(*token_offset).type != TokenType::MAP_BRACE_END) {
        assert(tokens.at(*token_offset).type == TokenType::STRING_TYPE);
        auto& key = tokens.at(*token_offset).value;
        *token_offset += 1;
        assert(*token_offset < tokens_len);
        assert(tokens.at(*token_offset).type == TokenType::COLON);
        *token_offset += 1;
        assert(*token_offset < tokens_len);
        jmap.emplace(std::move(key), Parse(token_offset));
        assert(*token_offset < tokens_len);
        if (tokens.at(*token_offset).type == TokenType::COMMA) {
          *token_offset += 1;
          assert(*token_offset < tokens_len);
        }
      }
      *token_offset += 1;
      return jmap;
    }
    default: assert(false);
  }
}

Json ParseJson(const std::string& json_str) {
  JsonParser json_parser(json_str);
  auto& tokens = json_parser.Tokenize();
  // std::cout << "Tokens(" << tokens.size() << "):" << std::endl;
  // for (auto& t : tokens) {
  //   std::cout << int(t.type) << " - '" << t.value << "'" << ", ";
  // }
  // std::cout << std::endl;
  return json_parser.Parse();
}

}  // namespace quick
