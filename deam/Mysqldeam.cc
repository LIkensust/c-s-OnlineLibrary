#include "../include/database/mysql_operation.h"
#include <iostream>
using namespace std;
int main() {
  auto tool = MysqlTool::make();
  tool->do_connect("../config/mysql_cnf.js");
  tool->interactive("insert into label values (1,'中文'),(2,'外文')\
                    ,(3,'文学'),(4,'教材'),\
                    (5,'参考书'),(6,'图册'),(7,'其他')");

//  tool->interactive("insert into label(label_meaning) values('中文')");
  int a = tool->interactive("select * from label");
  if(a == false) {
    cout<<"err"<<endl;
    return 0;
  }
  tool.get_deleter();
  auto row = tool->get_slution_by_row();
  while(row != NULL) {
    cout<<row[0]<<" "<<row[1]<<endl;
    row = tool->get_slution_by_row();
  }
}
