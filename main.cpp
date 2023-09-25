#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    std::fstream fstream;
    fstream.open("%appdata%\\Local\\Google\\Chrome\\User Data\\");
}