#include <iostream>

int main()
{
    int times = 10;
    char start = 'A';
    char start2 = 'a';
    for (int i = 0; i < times; i++)
    {
        std::cout << "{\"Numpad" << i ;
        std::cout << "\",XK_KP_" << i << "}," << std::endl;
    }
    return 0;
}