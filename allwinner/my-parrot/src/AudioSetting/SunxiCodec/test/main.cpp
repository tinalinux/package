#include <iostream>
using namespace std;
class A {
public:
    virtual void aa(){
        cout << "aa" << endl;
    }
    virtual void aaa() = 0;
};

class B : public A
{
    public:
        void aa(){
            cout << "bb" << endl;
        }
        void aaa(){
            cout << "bbb" << endl;
        }
};

int main()
{
    B *b = new B();
    b->aa();
    b->aaa();

    A *a = b;
    a->aa();
    a->aaa();
}
