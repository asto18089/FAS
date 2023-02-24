#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <string>

using std::cout;
using std::endl;
using std::vector;
using std::string;

vector <unsigned int[4]>get_flashrate_list() {
    vector <unsigned int[4]>result;
    static unsigned int data[4];
    /* id width height fps */
    
    FILE *dumpsys = popen("dumpsys display | grep \'DisplayModeRecord\'", "r");
    
    char buffer[1024] = {0};
    while (fgets(buffer, sizeof(buffer), dumpsys)) {
        static string analyze;
        analyze = buffer;
        
        analyze.pop_back();
        
        for (size_t i = 0; i < 4, i++) {
            switch (i) {
                case 0:
                    auto pos_id = analyze.find("id=");
                    auto pos_end = analyze.find(",", pos_id);
                    data[i] = atoi(analyze.substr(pos_id + 3, pos_end - pos_id - 3));
                    break;
                case 1:
                    auto pos_id = analyze.find("width=");
                    auto pos_end = analyze.find(",", pos_id);
                    i = atoi(analyze.substr(pos_id + 6, pos_end - pos_id - 6));
                    break;
                case 2:
                    auto pos_id = analyze.find("height=");
                    auto pos_end = analyze.find(",", pos_id);
                    i = atoi(analyze.substr(pos_id + 7, pos_end - pos_id - 7));
                    break;
                case 3:
                    auto pos_id = analyze.find("fps=");
                    auto pos_end = analyze.find(",", pos_id);
                    i = atoi(analyze.substr(pos_id + 7, pos_end - pos_id - 4));
            }
        }
        
        result.push_back(data);
    }
}