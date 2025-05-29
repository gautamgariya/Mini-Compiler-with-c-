#include <iostream>
#include <string>

using namespace std;  

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}


void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


void printChar(char c) {
    cout << "Character: " << c << endl;
}

int main() {

    int x = 5;
    float y = 3.14159;
    char c = 'A';
    char escaped_char = '\n';
    string str = "Hello, World!\n";

    int* ptr = &x;
    cout << "Value of x through pointer: " << *ptr << endl;
    

    x++;
    cout << "After increment: " << x << endl;
    x--;
    cout << "After decrement: " << x << endl;
    

    x += 5;
    y *= 2;
    

    if (x >= 10) {
        cout << "x is greater than or equal to 10" << endl;
    } else if (x > 5) {
        cout << "x is greater than 5 but less than 10" << endl;
    } else {
        cout << "x is less than or equal to 5" << endl;
    }

    for (int i = 0; i < 5; ++i) {
        cout << "Loop iteration " << i << ": " << x + i << endl;
    }
    

    int count = 0;
    while (count < 3) {
        cout << "Count: " << count << endl;
        count++;
    }
    

    int a = 10, b = 20;
    cout << "Before swap: a = " << a << ", b = " << b << endl;
    swap(&a, &b);
    cout << "After swap: a = " << a << ", b = " << b << endl;
 
    printChar('X');
    printChar('\t');
    

    cout << "String with escape: " << "First line\nSecond line\tTabbed" << endl;

    cout << "Factorial of 5: " << factorial(5) << endl;
    
  
    bool condition1 = true;
    bool condition2 = false;
    if (condition1 && !condition2) {
        cout << "Logical AND and NOT working" << endl;
    }
    
    if (condition1 || condition2) {
        cout << "Logical OR working" << endl;
    }
    
    return 0;
} 