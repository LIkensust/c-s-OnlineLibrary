#include "../src/include/database/mysql_operation.h"
#include <iostream>
using namespace std;
int main() {
  auto tool = MysqlTool::make();
  tool->do_connect("../config/mysql_cnf.js");
  tool->interactive("delete from bookinf");
  tool->interactive(
      "insert into bookinf values(1,'The C Programming Language','Dennis Ritchie','The C Program\
ming Language译作《C程序设计语言》，是\
由著名的计算机科学家Brian W. Kernighan\
和C语言之父的Dennis M. Ritchie合著的一\
部计算机科学著作，是第一部介绍C语言编程\
方法的书籍。它是一本必读的程序设计语言方\
面的参考书。它在C语言的发展和普及过程中\
起到了非常重要的作用，被视为是C语言的业\
界标准规范，而且至今仍然广泛使用。它也被\
公认为计算机技术著作的典范，以清晰简洁的\
文字讲述而著称。书中用hello world为实\
例开始讲解程序设计，也已经成为程序设计语言\
图书的传统。',3,NULL,NULL)");
  int a = tool->interactive("select * from bookinf");
  if (a == false) {
    cout << "err" << endl;
    return 0;
  }
  tool.get_deleter();
  auto row = tool->get_slution_by_row();
  while (row != NULL) {
    cout << row[0] << " " << row[1] << " " << row[2] << endl
         << row[3] << endl
         << row[4];
    row = tool->get_slution_by_row();
  }
}
