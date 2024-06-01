#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <string_view>
#include <algorithm>

using namespace std::literals;
struct JsonArray;
struct JsonObject;

template <typename T> struct recursive_wrapper {
    recursive_wrapper(T t_){
        t.emplace_back(std::move(t_));
    }

    operator const T&() const {
        return t.front();
    }
    std::vector<T> t;
};

using JsonValue = std::variant<std::monostate, std::string, double, bool, nullptr_t,
    recursive_wrapper<JsonArray>, recursive_wrapper<JsonObject>>;

struct JsonArray {
    std::vector<JsonValue> arr;
    void push_back(const JsonValue& v){ arr.push_back(v); };
};

struct JsonObject {
    std::unordered_map<std::string, JsonValue> objs;
    JsonValue& operator[](const std::string& key){
        return objs[key];
    };
};


class JsonParser {
public:
    explicit JsonParser(const std::string& json) : json_(json), pos_(0) {}

    JsonValue parse() {
        skipWhitespace();
        auto value = parseObject();
        skipWhitespace();
        if (pos_ != json_.size()) {
            throw std::runtime_error("Extra characters found after parsing JSON" + std::to_string(pos_));
        }
        return value;
    }

private:
    const std::string_view json_;
    size_t pos_;

    void skipWhitespace() {
        while (pos_ < json_.size() && std::isspace(json_[pos_])) {
            ++pos_;
        }
    }

    JsonValue parseValue() {
        skipWhitespace();
        if (pos_ >= json_.size()) throw std::runtime_error("Unexpected end of input");

        switch (json_[pos_]) {
            case 'n': return parseLiteral("null"sv, nullptr);
            case 't': return parseLiteral("true"sv, true);
            case 'f': return parseLiteral("false"sv, false);
            case '"': return parseString();
            case '[': return parseArray();
            case '{': return parseObject();
            default: return parseNumber();
        }
    }

    JsonValue parseLiteral(const std::string_view& literal, JsonValue value) {
        if (json_.compare(pos_, literal.size(), literal) == 0) {
            pos_ += literal.size();
            return value;
        }
        throw std::runtime_error("Invalid literal");
    }

    JsonValue parseString() {
        if (json_[pos_] != '"') throw std::runtime_error("Expected '\"'");
        ++pos_;
        std::string result;
        while (pos_ < json_.size() && json_[pos_] != '"') {
            if (json_[pos_] == '\\') {
                ++pos_;
                if (pos_ >= json_.size()) throw std::runtime_error("Invalid escape sequence");
                switch (json_[pos_]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw std::runtime_error("Invalid escape character");
                }
            } else {
                result += json_[pos_];
            }
            ++pos_;
        }
        if (pos_ >= json_.size() || json_[pos_] != '"') throw std::runtime_error("Expected '\"'");
        ++pos_;
        return result;
    }

    JsonValue parseNumber() {
        size_t start_pos = pos_;
        while (pos_ < json_.size() && (std::isdigit(json_[pos_]) || json_[pos_] == '.' || json_[pos_] == '-')) {
            ++pos_;
        }
        std::string_view num_str = json_.substr(start_pos, pos_ - start_pos);
        return std::stod(std::string(num_str));
    }

    JsonValue parseArray() {
        if (json_[pos_] != '[') throw std::runtime_error("Expected '['");
        ++pos_;
        JsonArray array;
        skipWhitespace();
        if (json_[pos_] == ']') {
            ++pos_;
            return array;
        }
        while (true) {
            array.push_back(parseValue());
            skipWhitespace();
            if (json_[pos_] == ']') {
                ++pos_;
                break;
            }

            if (json_[pos_] != ',') throw std::runtime_error("Expected ','" + std::to_string(pos_));
            ++pos_;
            skipWhitespace();
        }
        return array;
    }

    JsonValue parseObject() {
        if (json_[pos_] != '{') throw std::runtime_error("Expected '{'");
        ++pos_;
        JsonObject object;
        skipWhitespace();
        if (json_[pos_] == '}') {
            ++pos_;
            return object;
        }
        while (true) {
            skipWhitespace();
            std::string key = std::get<std::string>(parseString());
            skipWhitespace();
            if (json_[pos_] != ':') throw std::runtime_error("Expected ':'");
            ++pos_;
            skipWhitespace();
            object[key] = parseValue();
            skipWhitespace();
            if (json_[pos_] == '}') {
                ++pos_;
                break;
            }
            
            if (json_[pos_] != ',') throw std::runtime_error("Expected ','" + std::to_string(pos_));
            ++pos_;
            skipWhitespace();
        }
        return object;
    }
};

struct prettyJson {
    mutable int level = 0;
    void printIndent() const{
        for(auto i=0; i<level*4; i++){
            std::cout << " ";
        }
    }
    void operator()(std::monostate) const {}
    void operator()(const std::string& str) const { 
        std::cout << "\"" << str << "\""; 
    }
    void operator()(nullptr_t) const { 
        std::cout << "null"; 
    }
    void operator()(bool b) const { 
        std::cout << (b ? "true" : "false");
    }
    void operator()(double d) const { 
        std::cout << d; 
    }
    void operator()(JsonArray ja) const {
        std::cout << "[";
        if(!ja.arr.empty()){
            level++;
            std::cout << "\n";
            printIndent();
            auto it = ja.arr.begin();
            std::visit(*this, *it);
            std::for_each(++it, ja.arr.end(), [this](const auto &v){
                std::cout << ",\n";
                printIndent();
                std::visit(*this, v);
            });
            std::cout << "\n";
            level--;
        }
        printIndent();
        std::cout << "]";
    }

    void operator()(JsonObject jo) const {
        printIndent();
        std::cout << "{";
        if(!jo.objs.empty()){
            std::cout << "\n";
            level++;
            printIndent();
            auto it = jo.objs.begin();
            const auto &[k, v] = *it;
            std::cout << '"' << k << '"' << ": ";
            std::visit(*this, v);

            std::for_each(++it, jo.objs.end(), [this](const auto &v){
                std::cout << ",\n";
                printIndent();
                std::cout << '"' << v.first << '"' << ": ";
                std::visit(*this, v.second);
            });
            std::cout << "\n";
            level--;
        }
        printIndent();
        std::cout << "}\n";
    }
};

void printJsonValue(const JsonValue& value) {
    std::visit(prettyJson{} ,value);
}
