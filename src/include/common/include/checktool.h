#pragma once
#include "common.h"
#include <iostream>
#include <regex.h>

constexpr int DEFAULTSIZE = 5;

class RegexTool {
public:
  bool set_regex(std::string regex) {
    regfree(&reg_);
    regex_ = regex;
    if (regcomp(&reg_, regex_.c_str(), REG_EXTENDED | REG_NEWLINE) != 0)
      return false;
    return true;
  }

  static std::unique_ptr<RegexTool> make() {
    return std::unique_ptr<RegexTool>(new RegexTool);
  }

  RegexTool &resize(int newsize) {
    ASSERT_MSG(newsize >= 1, "Can't resize to thie size.");
    result_size_ = newsize;
    result_.reset(new regmatch_t[newsize]);
    return *this;
  }

  ~RegexTool() {
    regfree(&reg_);
    result_.reset();
  }

  std::vector<std::pair<int, int>> check_str(const char *src) {
    if (src == NULL) {
      return std::vector<std::pair<int, int>>();
    }
    int reg_ret = regexec(&reg_, src, result_size_, result_.get(), 0);
    if (reg_ret == REG_NOMATCH) {
      return std::vector<std::pair<int, int>>();
    } else if (reg_ret != 0) {
      ASSERT_MSG(false, "RegexTool err");
    }
    std::vector<std::pair<int, int>> ret;
    for (int i = 0; static_cast<size_t>(i) < result_size_; i++) {
      if (result_.get()[i].rm_so == -1)
        break;
      ret.push_back({result_.get()[i].rm_so, result_.get()[i].rm_eo});
    }
    return ret;
  }

protected:
  RegexTool() : result_size_(DEFAULTSIZE) {
    bzero(&reg_, sizeof(regex_t));
    result_.reset(new regmatch_t[DEFAULTSIZE]);
  }

  std::string regex_;
  regex_t reg_;
  std::shared_ptr<regmatch_t> result_;
  size_t result_size_;
};
