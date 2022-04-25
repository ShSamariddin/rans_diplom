//
// Created by Samariddin Sharipov on 19.04.2022.
//
#include <iostream>
class samar{
public:
    void f(){
        year *= 2;
    }

    int year;
    int weight;
    int hight;
};
int main(){
    samar a;
    a.hight = 4;
    a.year = 2;
    a.f();
    std::cout << a.year;
}