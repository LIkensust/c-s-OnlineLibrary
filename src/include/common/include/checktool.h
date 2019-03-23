#pragma once
#include "../common.h"
#include <iostream>
#include <regex.h>

class RegexTool {
public:
  bool set_regex(std::string regex) {
    regex_ = regex;
    if (regcomp(&reg_, regex_.c_str(), 0) != 0)
      return false;
    return true;
  }

protected:
  static std::unique_ptr<RegexTool> make() {
    return std::unique_ptr<RegexTool>(new RegexTool);
  }
  std::string regex_;
  regmatch_t *pmatch;
  regex_t reg_;
  //记录结果
  //释放结果数组
  //支持对数组resize
};

bool check_str(const char *stc, const char *regex) {
  if (stc == NULL)
    return false;
  return false;
}
