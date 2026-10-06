#include <any>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
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

int extract_array(std::string_view in, int &cur);

std::string extract_string_value(std::string_view in, int &cur) {
  std::string result;
  int i = 1;

  while (in[i] != EOF) {
    char c = in[i];

    if (c == '"') {
      cur += i;
      return result;
    }

    result.push_back(c);
    i++;
  }

  perror("formato incorrecto");
  exit(1);
}

double extract_numeric_value(std::string_view in, int &cur) {
  std::string result;
  int i = 0;
  char next_char = in[i + 1];

  if (in[i] == '-') {
    result.push_back('-');
    i++;
  }

  while (std::isdigit(in[i]) || in[i] == '.' || in[i] == 'e' || in[i] == '-') {
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

    if (c == '[') {
      std::cout << "array: " << std::endl;
      extract_array(&in[i], i);
    }

    if (std::isdigit(c) || c == '-') {
      std::cout << "value: " << extract_numeric_value(&in[i], i) << std::endl;
    }

    if (c == '"') {
      std::cout << "value: " << extract_string_value(&in[i], i) << std::endl;
    }

    if (c == 'n') {
      std::cout << "Null!" << std::endl;
    }

    if (c == 't') {
      std::cout << "True" << std::endl;
    }

    if (c == 'f') {
      std::cout << "False" << std::endl;
    }

    return i;
  }

  perror("formato de clave invalido");
  exit(1);
}

int extract_object(std::string_view in) {
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

    if ((c == '"' || c == '-' || isdigit(c) || c == '[') && !in_key) {
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

int extract_array(std::string_view in, int &cur) {
  bool is_open = false;
  bool comma = false;

  for (int i = 0; i < in.length(); i++) {
    char c = in[i];

    if (c == '[' && !is_open) {
      std::cout << "init array" << std::endl;
      is_open = true;
      continue;
    }

    if (c == '[' && is_open) {
      std::cout << "new array" << std::endl;
      i += extract_array(&in[i], i);
      continue;
    }

    if (std::isspace(c) || c == '\t' || c == '\n') {
      continue;
    }

    if ((c == '"' || c == '-' || isdigit(c))) {
      std::cout << "extract value" << std::endl;
      i += get_value(&in[i]);
      comma = false;
      continue;
    }

    if (c == ',') {
      comma = true;
      continue;
    }

    if (c == ']') {

      if (comma) {
        perror("error de formato");
        exit(1);
      }

      std::cout << "array finalized" << std::endl;
      is_open = false;
      cur += i;
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
  std::fstream stream("input.json");

  if (!stream.is_open()) {
    perror("error al abrir el archivo\n");
    exit(0);
  }

  std::streamsize size = stream.rdbuf()->in_avail();
  char content[size + 1];
  stream.read(content, size);
  content[size] = '\0';

  extract_object(content);

  /** Propuesta de api
  Hello test = json.map<Hello>();
  std::cout << test.hello << std::endl;*/

  return 0;
}
