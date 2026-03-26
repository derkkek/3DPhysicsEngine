#include "raylib.h"
#include "Program.h"
#include <iostream>

int main(void)
{
    Program program;

    program.Update();

    program.Destroy();
   
    std::cout << "git check";
    return 0;
}