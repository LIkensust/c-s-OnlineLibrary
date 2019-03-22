#include "../include/database/mysql_operation.h"
#include <iostream>
using namespace std;
int main() {
  auto tool = MysqlTool::make();
  tool->do_connect("../config/mysql_cnf.js");
  tool->interactive("insert into label(label_meaning) values \
                    (\"中文\"),(\"外文\"),(\"文学\"),(\"教材\"),\
                    (\"参考书\"),(\"图册\"),(\"其他\")");
}
