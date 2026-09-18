#include <any>
#include <cctype>
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

double extract_numeric_value(std::string_view in, int &cur) {
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

int get_key(std::string_view in) {
  bool open = false;

  std::cout << "key: ";
  for (int i = 0; i < in.length(); i++) {
    char c = in[i];

    if (c == ' ' || c == '\n') {
      continue;
    }

    if (c == '"') {
      if (open) {
        std::cout << '\n';
        return i;
      } else {
        open = true;
        continue;
      }
    }

    std::cout << c;
  }

  perror("formato de clave invalido");
  exit(1);
}

int get_value(std::string_view in) {
  for (int i = 0; i < in.length(); i++) {
    char c = in[i];

    if (c == ' ' || c == '\n') {
      continue;
    }

    if (std::isdigit(c) || c == '-') {
      std::cout << "value: " << extract_numeric_value(&in[i], i) << std::endl;
      return i;
    }

    if (c == '"') {
      std::cout << "String!" << std::endl;
    }
  }

  perror("formato de clave invalido");
  exit(1);
}

int extract_object(std::string_view in) {
  std::cout << in << std::endl;

  bool is_open = false;
  bool in_key = true;

  for (int i = 0; i < in.length(); i++) {
    char c = in[i];

    if (c == '{' && !is_open) {
      std::cout << "init object" << std::endl;
      is_open = true;
      continue;
    }

    if (c == '{' && is_open) {
      std::cout << "new object" << std::endl;
      i += extract_object(&in[i]);
      in_key = true;
      continue;
    }

    if (c == '"' && in_key) {
      std::cout << "extract key" << std::endl;
      in_key = false;
      i += get_key(&in[i]);
      continue;
    }

    if (c == ':' && in_key) {
      perror("error de formato");
      exit(1);
    }

    if (c == ':' && !in_key) {
      std::cout << "separator ':'" << std::endl;
      continue;
    }

    if ((c == '"' || c == '-' || isdigit(c)) && !in_key) {
      std::cout << "extract value" << std::endl;
      i += get_value(&in[i]);
      in_key = true;
      continue;
    }

    if (c == '}') {
      std::cout << "object finalized" << std::endl;
      is_open = false;
      return i;
    }
  }

  if (is_open) {
    perror("error de formato");
    exit(1);
  }

  return -1;
}

int main() {
  std::fstream file("input.json");

  extract_object("{\"key\": -10, \"key\": {}}");

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

  if (tokens.size() > 0) {
    perror("formato incorrecto");
    exit(-1);
  }
  return 0;
}
