/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <minizinc/astexception.hh>
#include <minizinc/model.hh>

#include <fstream>
#include <string>
#include <vector>

namespace MiniZinc {

class JSONError : public LocationException {
public:
  JSONError(EnvI& env, const Location& loc, const std::string& msg)
      : LocationException(env, loc, msg) {}
  const char* what() const throw() override { return "JSON parsing error"; }
};

class JSONValue {
public:
  enum Type { OBJECT, ARRAY, NUMBER, STRING, BOOL, NIL } type;
  std::map<std::string, JSONValue> objectValue;
  std::vector<JSONValue> arrayValue;
  double numberValue;
  std::string stringValue;
  bool boolValue;

  JSONValue() : type(NIL), numberValue(0), boolValue(false) {}

  template <typename T>
  std::vector<T> getVec() const {
    if (type != ARRAY) {
      throw std::runtime_error("JSONValue is not an array.");
    }

    std::vector<T> result;
    for (const auto& item : arrayValue) {
      if constexpr (std::is_same<T, std::string>::value) {
        if (item.type == STRING) {
          result.push_back(item.stringValue);
        } else {
          throw std::runtime_error("Type mismatch in array.");
        }
      } else if constexpr (std::is_same<T, double>::value) {
        if (item.type == NUMBER) {
          result.push_back(item.numberValue);
        } else {
          throw std::runtime_error("Type mismatch in array.");
        }
      } else if constexpr (std::is_same<T, bool>::value) {
        if (item.type == BOOL) {
          result.push_back(item.boolValue);
        } else {
          throw std::runtime_error("Type mismatch in array.");
        }
      } else {
        throw std::runtime_error("Unsupported type.");
      }
    }
    return result;
  }
};

class JSONParser {
protected:
  enum TokenT {
    T_LIST_OPEN,
    T_LIST_CLOSE,
    T_OBJ_OPEN,
    T_OBJ_CLOSE,
    T_COMMA,
    T_COLON,
    T_STRING,
    T_INT,
    T_FLOAT,
    T_BOOL,
    T_NULL,
    T_EOF
  } _t;

  EnvI& _env;
  int _line;
  int _column;
  std::string _filename;
  Location errLocation() const;
  class Token;
  Token readTokenInternal(std::istream& is);
  Token readToken(std::istream& is);
  void expectToken(std::istream& is, TokenT t);
  std::string expectString(std::istream& is);
  int expectInt(std::istream& is);
  void expectEof(std::istream& is);
  Expression* parseEnum(std::istream& is);
  Expression* parseEnumObject(std::istream& is, const std::string& seen);
  Expression* parseExp(std::istream& is, bool parseObjects = true, TypeInst* ti = nullptr);
  Expression* parseArray(std::istream& is, TypeInst* ti = nullptr, size_t range_index = 0);
  Expression* parseSet(std::istream& is, TypeInst* ti = nullptr);
  Expression* parseObject(std::istream& is, TypeInst* ti = nullptr);

  void parseModel(Model* m, std::istream& is, bool isData);

public:
  JSONParser(EnvI& env) : _env(env) {}
  /// Parses \a filename as MiniZinc data and creates assign items in \a m
  void parse(Model* m, const std::string& filename, bool isData = true);
  /// Parses \a data as JSON-encoded MiniZinc data and creates assign items in \a m
  void parseFromString(Model* m, const std::string& data, bool isData = true);
  /// Check if file \a filename may contain JSON-encoded MiniZinc data
  static bool fileIsJSON(const std::string& filename);
  /// Check if string \a data may contain JSON-encoded MiniZinc data
  static bool stringIsJSON(const std::string& data);
  /// Coerces a array literal to take shape and (tuple) type
  Expression* coerceArray(TypeInst* intendedTI, ArrayLit* al);
  JSONValue parseObject2(std::istream& is);
  JSONValue parseArray2(std::istream& is);
  JSONValue parseNumber2(std::istream& is);
  JSONValue parseValue2(std::istream& is);
};
}  // namespace MiniZinc