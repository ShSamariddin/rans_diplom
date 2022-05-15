//
// Created by Samariddin Sharipov on 30.04.2022.
//

#include <iostream>
#include "pgm/pgm.h"

using namespace  std;
int main()
{
    string file1, file2;
    cin>> file1>> file2;
    PGM pic1;
    PGM pic2;
    pic1.read(file1);
    pic2.read(file2);
    for(int i = 0; i < 10; i ++){
        std::cout << pic1.data[i]<<' '<<pic2.data[i]<<'\n';
    }
    return 0;
}