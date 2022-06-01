#include <iostream>
#include "skiplist.h"

#define FILE_PATH "./store/dumpFile"

int main(){
    SkipList<std::string, std::string> skipList(10);
    SkipList<std::string, std::string> skipList2(10);

    skipList.insert_element("1", "a");
    skipList.change_element("1", "A");
    skipList.insert_element("3", "b");
    skipList.insert_element("7", "c");
    skipList.insert_element("8", "gzhu");
    skipList.insert_element("9", "lys");
    skipList.insert_element("19", "wtf");
    skipList.dump_file();
    skipList2.load_file();

    std::cout << "skipList size:" << skipList.size() << std::endl;

   skipList.search_element("9");
   skipList.search_element("18");


   skipList.delete_element("3");
   skipList.delete_element("7");

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();

    std::cout << "skipList size:" << skipList2.size() << std::endl;

    skipList2.display_list();
    return 0;
}