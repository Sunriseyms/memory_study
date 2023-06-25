#include <ext/pool_allocator.h>
#include <iostream>
#include <memory>

using namespace std;

/**
 * Foo* p = new Foo(x);
 * delete p;
 *
 * 等同于
 * Foo* p = (Foo*)operator new(sizeof(Foo)); // 这里是可以重载的
 * new(p)Foo(x);
 * p->~Foo()
 * operator delete(p); // 这里也是可以重载的
 * **/

void *myAlloc(size_t size) { return malloc(size); }

void myFree(void *ptr) { return free(ptr); }

inline void *operator new(size_t size) {
  cout << "yms global new(), \t" << size << endl;
  return myAlloc(size);
}

inline void *operator new[](size_t size) {
  cout << "yms global new[](), \t" << size << endl;
  return myAlloc(size);
}

inline void operator delete(void *ptr) {
  cout << "yms global delete(ptr), \t" << ptr << endl;
  return myFree(ptr);
}

inline void operator delete[](void *ptr) {
  cout << "yms global delete[](ptr), \t" << ptr << endl;
  return myFree(ptr);
}

class B {
 public:
  int id;
  B() : id(0) {
    cout << "B default ctor. this=" << this << " id=" << id << endl;
  }
  B(int i) : id(i) { cout << "B ctor. this=" << this << " id=" << id << endl; }
  ~B() { cout << "B dtor. this=" << this << " id=" << id << endl; }

  void operator delete[](void *p, size_t size) {
    cout << "B::operator delete[] " << p << " " << size << endl;
    ::operator delete[](p);
  }

  void *operator new(size_t size, void *loc) {
    cout << "B placement new size=" << size << endl;
    return loc;
  }

  void *operator new(size_t size) {
    cout << "B operator new size=" << size << endl;
    return ::operator new(size);
  }

  void *operator new[](size_t size) {
    cout << "B operator new[](), \t" << size << endl;
    return myAlloc(size);
  }

  // void operator delete(void* p, void* p1) {
  //  cout << "B placement delete[] " << endl;
  //}
};

class A : public B {
 public:
  int id;

  A() : id(0) {
    cout << "A default ctor. this=" << this << " id=" << id << endl;
  }
  A(int i) : B(i), id(i) {
    cout << "A ctor. this=" << this << " id=" << id << endl;
  }
  ~A() { cout << "A dtor. this=" << this << " id=" << id << endl; }
  void operator delete[](void *p, size_t size) {
    cout << "A::operator delete[] " << p << " " << size << endl;
  }
};

void test1() {
  cout << "sizeof(A)=" << sizeof(A) << endl;
  int size = 3;
  A *buf = new A[3];
  A *tmp = buf;
  cout << "buf=" << buf << " tmp=" << tmp << endl;

  for (int i = 0; i < size; ++i) {
    new (tmp++) A(i);
  }
  cout << "buf=" << buf << " tmp=" << tmp << endl;

  delete[] buf;
}

void test2() {
  A *pA = new A[3];
  cout << pA << endl;
  cout << *((size_t *)pA - 1) << endl;
  delete[] pA;

  A *pA1 = new A[1];  // has counter
  cout << *((size_t *)pA1 - 1) << endl;
  delete[] pA1;

  A *pA2 = new A;  // no counter
  cout << *((size_t *)pA2 - 1) << endl;
  delete pA2;

  B *pB = new B[5];
  cout << pB << endl;
  cout << *((size_t *)pB - 1) << endl;
  delete[] pB;
}

void test3() {
  B *pB = new B[3];
  delete
      [] pB;  // delete
              // pB会crash，错在与pB指向前面有一个counter的内存区域，如果强制按照B的结构去delete会失败

  B b[3];  // b在栈上，就不会涉及到还要一个counter的内存区域(这是为了管理堆内存产生的)
  cout << "sizeof(b)=" << sizeof(b) << endl;
}

void test4() {
  int size = 3;
  char *buf = new char[sizeof(B) * size];
  cout << "buf=" << buf << " counter=" << *((size_t *)buf - 1)
       << endl;  // no counter
  char *tmp = buf;
  for (int i = 0; i < size; ++i) {
    B *pB = new (tmp) B(i);
    tmp = tmp + sizeof(B);
    pB->~B();  // no placement delete, 只有在构造函数异常的时候，才可能需要调用
  }

  delete[] buf;  // no dtor call
}

void test5() {
  B *pb = new B;
  delete pb;

  B *pb1 = new B[3];
  delete[] pb1;
}


int main() {
  void *p1 = malloc(512);
  free(p1);

  int *p2 = new int;
  delete p2;

  void *p3 = ::operator new(512);
  ::operator delete(p3);

  int *p4 = std::allocator<int>().allocate(5);  // 5个ints
  std::allocator<int>().deallocate(p4, 5);

#ifdef __GNUC__
  void *p5 = __gnu_cxx::__pool_alloc<int>().allocate(9);
  __gnu_cxx::__pool_alloc<int>().deallocate((int *)p5, 9);
#endif

  cout << "===================test1 begin=========================" << endl;
  test1();
  cout << "===================test1 end=========================" << endl;

  cout << "===================test2 begin=========================" << endl;
  test2();
  cout << "===================test2 end=========================" << endl;

  cout << "===================test3 begin=========================" << endl;
  test3();
  cout << "===================test3 end=========================" << endl;

  cout << "===================test4 begin=========================" << endl;
  test4();
  cout << "===================test4 end=========================" << endl;

  cout << "===================test5 begin=========================" << endl;
  test5();
  cout << "===================test5 end=========================" << endl;
}