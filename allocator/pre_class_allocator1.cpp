#include <cstddef>
#include <iostream>

using namespace std;

class Screen {
 public:
  Screen(int x) : i(x) {}
  int get() const { return i; }

  void* operator new(size_t);
  void operator delete(void*, size_t);

 private:
  Screen* next; // 这里会存在一个额外的指针，对于每个对象都会增加一个
  static Screen* freeStore;
  static const int screenChunk;
  int i;
};

// 通过重载operator new/delete来进行内存管理，统一进行分配内存；
// 节省内存，可以省去counter及相关的debug信息；减少malloc申请内存次数，一定程度的减少system
// call，减少page fault发生率
Screen* Screen::freeStore = 0;
const int Screen::screenChunk = 24;
void* Screen::operator new(size_t size) {
  Screen* p;
  if (!freeStore) {
    size_t chunk = screenChunk * size;
    freeStore = p = reinterpret_cast<Screen*>(malloc(chunk)); // 统一申请内存
    for (; p != &freeStore[screenChunk - 1]; ++p) { // 将内存使用单向链表管理起来
      p->next = p + 1;
    }
    p->next = 0;
  }

  p = freeStore; // 分配内存
  freeStore = freeStore->next; // 指向空闲链表的表头
  return p;
}

// 标准的链表回收机制
void Screen::operator delete(void* p, size_t) {
  (static_cast<Screen*>(p))->next = freeStore; // 将不使用的内存，放回链表(表头)
  freeStore = static_cast<Screen*>(p); // 将freeStore指向最新的链表头
}

int main() {
  cout << sizeof(Screen) << endl;
  size_t const N = 100;
  Screen* p[N];
  for (int i = 0; i < N; ++i) {
    p[i] = new Screen(i);
  }

  for (int i = 0; i < 10; ++i) {
    cout << p[i] << endl;
  }

  for (int i = 0; i < N; ++i) {
    delete p[i];
  }
}
