#ifndef _Z_DTWC_H_
#define _Z_DTWC_H_
/**@file zit/utility/dtwc.h
 * @brief Diabolic tricks and wicked craft
 * @note
 *  OO 1. Single-Responsibility Principle; a object one thing cause modify;
 *     2. Open-Close Principle: module can extern, not modify;
 *     3. Dependency-Inversion Principle: abstract not dependence detail;
 *     4. Liskov-Substituent Principle: subclass replace parent class;
 *     5. Interface-Segregation Principle: One interface do one thing;
 */

inline void zstack_order(){
    int g = 1;
    printf("g=1;printf(g:%d, ++g:%d)\n", g, ++g);
    printf("g=2;printf(g:%d, g++:%d)\n", g, g++);
}

inline int zf2i(float f){
    return (int&)f;
}

inline int zremov_1(int x){
    return x&(x-1);
}

inline int zaverage(int x, int y){
    //return (x+y)>>1;
    return ((x&y)+((x^y)>>1));
}

inline void zswap(int *a, int *b){
    *a^=*b;
    *b^=*a;
    *a^=*b;
}

#define STRU_OFFSET(struc, member) (size_t)&(((struc*)0)->member)

#if 0
inline int zadd(int a, int b){
    int sub,carry;
    if(0==b)return a;
    sum = a^b;
    carry = a&b<<1;
    return zadd(sum, carry);
}

/* C++ CRTP(Curiously Recurring Template Pattern)
 * Eigen lib
 * [1] https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Curiously_Recurring_Template_Pattern
 * [2] https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 */
#include<iostream>
#include<stddef.h>
using namespace std;
template<class Derived>
struct Base  {
    void Interface(){
        static_cast<Derived*>(this)->Implementation();
    }
    static void StaticInterface()      {          // 编译期将绑定至子类方法
        Derived::StaticImplementation();
    }
    void Implementation(){
        cout <<"Base Implementation"<<endl;
        return;
    }
    static void StaticImplementation(){
        cout << "Base StaticImplementation"<<endl;
        return;
    }
};
// The Curiously Recurring Template Pattern (CRTP)
struct Derived1 : Base<Derived1>  {
    static void StaticImplementation(){
        cout<<"Derived1::StaticImplementatino()"<<endl;
    }
};
struct Derived2 : Base<Derived2>  {
    void Implementation(){
        cout<<"Derived2::Implementatin()"<<endl;
    }
};

template<typename T>
void test(Base<T> &t){
    t.StaticImplementation();
    t.StaticInterface();
    t.Interface();
    t.Implementation();
}

int main(){
    Derived1 dev1;
    Derived2 dev2;

    cout<<"dev1.fun()"<<endl;
    dev1.StaticImplementation();
    dev1.StaticInterface();
    dev1.Interface();
    dev1.Implementation();

    cout<<"dev2.fun()"<<endl;
    dev2.StaticImplementation();
    dev2.StaticInterface();
    dev2.Interface();
    dev2.Implementation();

    cout<<"test(dev1)"<<endl;
    test(dev1);
    cout<<"test(dev2)"<<endl;
    test(dev2);
}
#endif

#endif
