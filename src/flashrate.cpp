#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <string>

using std::cout;
using std::endl;
using std::vector;
using std::string;

static 

vector <unsigned int[3]>get_flashrate_list() {
    static unsigned int data[3];
    
    FILE *dumpsys = popen("dumpsys display | grep \'DisplayModeRecord\'", "r");
    
    char buffer[1024] = {0};
    while (fgets(buffer, sizeof(buffer), dumpsys)) {
        static string analyze;
        analyze = buffer;
        
        
    }
}