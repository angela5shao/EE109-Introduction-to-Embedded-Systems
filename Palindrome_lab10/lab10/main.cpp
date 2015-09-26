/*
 Lab 10: C++ code for palindrome
 */

#include <iostream>
using namespace std;

int main(int argc, const char * argv[]) {
    string input;
    cout << "Enter up to 20 digits w/o space: ";
    cin >> input;
    
    bool Pal = true;
    for (int i=0; i<input.length()/2; i++) {
        if (input[i] != input[input.length()-i-1]) {
            cout << "Not palindrome" << endl;
            Pal = false;
            break;
        }
    }
    if (Pal)
        cout << "Your string: " << input << " IS a palindrome" << endl;
    
    
    char count[10] = {'0','0','0','0','0','0','0','0','0','0'};
    for (int j=0; j<input.length(); j++) {
        count[input[j]-'0']++;
        cout << endl;
    }
    cout << "count arr: " << endl;
    for (int k=0; k<10; k++)
        cout << count[k] - '0' << " ";
    cout << endl;
    
    return 0;
}
