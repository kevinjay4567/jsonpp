#include <any>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stack>
#include <string>
#include <string_view>

enum json_values { NUMBER, STRING, NOLL, ARRAY, OBJECT };

struct json_key {
  std::string_view value;
};

struct json_value {
  std::any value;
  json_values value_type;
};

double extract_numeric_value(std::string in, size_t &cur) {
  std::string result;
  int i = 0;

  if (in[i] == '-') {
    result.push_back('-');
    i++;
  }

  while (std::isdigit(in[i]) || in[i] == '.') {
    if (in[i] == '.' && !std::isdigit(in[i + 1])) {
      perror("formato incorrecto: ..");
      exit(0);
    }

    result.push_back(in[i]);
    i++;
  }

  cur += i;
  return std::stod(result);
}

std::string get_key(std::string_view in) {
  int index = in.find(":");
  std::string result;
  std::string clean_result;

  result.append(in, 0, index);

  for (char c : result) {
    if (c == ' ' || c == '\n' || c == '"') {
      continue;
    }

    clean_result.push_back(c);
  }

  return clean_result;
}

std::any get_value(std::string_view in) {
  size_t index = in.find(":");

  if (index == std::string::npos) {
    perror("Error de formato: ':'  missing");
  }

  std::string result;
  std::string clean_result;

  result.append(in, index + 1);

  for (size_t i = 0; i < result.length(); i++) {
    char c = result[i];

    if (c == ' ' || c == '\n') {
      continue;
    }

    if (std::isdigit(c) || c == '-') {
      return extract_numeric_value(result.substr(i), i);
    }

    if (c == '"') {
      std::cout << "String!" << std::endl;
    }

    if (c == '[') {
      std::cout << "Array!" << std::endl;
    }

    if (c == '{') {
      std::cout << "Object!" << std::endl;
    }

    clean_result.push_back(c);
  }

  return std::stoi(clean_result);
}

int main() {
  std::fstream file("input.json");

  if (!file.is_open()) {
    return 0;
  }

  char letter = file.get();
  std::stack<char> tokens;

  while (!file.eof()) {
    if (std::isspace(letter) || letter == '\t' || letter == '\n') {
      letter = file.get();
      continue;
    }

    if (letter == '{') {
      tokens.push(letter);
    }

    if (letter == '}') {
      if (tokens.size() == 0 || tokens.top() != '{') {
        perror("error formato: llave de apertura faltante '{'");
        exit(-1);
      } else {
        tokens.pop();
      }
    }

    if (letter == '"') {

      if (tokens.top() == '{') {
        tokens.push(letter);
      } else if (tokens.top() == ',') {
        tokens.pop();
        tokens.push(letter);
      } else {
        perror("error formato: token '\"' fuera de lugar");
        exit(-1);
      }

      letter = file.get();
      while (!file.eof()) {

        if (letter == '"') {
          tokens.pop();
          break;
        }

        std::cout << letter << std::endl;
        letter = file.get();
      }

      if (file.peek() == ':') {
        tokens.push(file.get());
        letter = file.get();
      }
    }

    if (letter == '-' || isdigit(letter)) {
      if (tokens.top() == ':') {

        std::cout << "Valor Numerico: ";
        while (!file.eof()) {

          if (letter == ',') {
            tokens.pop();
            break;
          }

          if (letter == ']' || letter == '}') {
            tokens.pop();
            tokens.pop();
            break;
          }

          std::cout << letter;
          letter = file.get();
        }
      }
    }

    letter = file.get();
  }

  /*std::string content;
  std::map<std::string, std::any> m{};

  while (std::getline(file, content)) {
    if (content.find('"') == std::string::npos) {
      continue;
    }

    m[get_key(content)] = get_value(content);
  }

  file.close();

  for (const auto &[key, value] : m) {
    std::cout << "KEY: [" << key << "]\n";
    std::cout << "TYPE: " << value.type().name() << "\n";
  }

  std::cout << std::any_cast<double>(m["age"]) << std::endl;*/

  if (tokens.size() > 0) {
    perror("formato incorrecto");
    exit(-1);
  }
  return 0;
}
