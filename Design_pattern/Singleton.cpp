#if 0
class Singleton {
    public:
        static Singleton* GetInstance() {
            if (_instance == nullptr) {
                _instance = new Singleton();
            }
            return _instance;
        }

    private:
        Singleton() {}
        ~Singleton() {}
        Singleton(const Singleton& clone) {}
        Singleton& operator=(const Singleton&) {}  //赋值操作
        static Singleton* _instance;  //一个类只有一个实例
};

Singleton* Singleton::_instance = nullptr;  //静态成员函数类内声明，类外初始化

#elif 0
// c++11 magic static 特性：如果当变量在初始化的时候，并发同时进⼊声明语句，并发线程将
// 会阻塞等待初始化结束。
// c++ effective
class Singleton
{
    public:
        static Singleton& GetInstance() {
            static Singleton instance;
            return instance;
        }

    private:
        Singleton();
        ~Singleton();
        Singleton(const Singleton& clone) {}
        Singleton& operator=(const Singleton&) {}
};

// 继承 Singleton
// // g++ Singleton.cpp -o singleton -std=c++11
/*该版本具备 版本5 所有优点：
  1. 利⽤静态局部变量特性，延迟加载；
  2. 利⽤静态局部变量特性，系统⾃动回收内存，⾃动调⽤析构函数；
  3. 静态局部变量初始化时，没有 new 操作带来的cpu指令reorder操作；
  4. c++11 静态局部变量初始化时，具备线程安全；
  */

#else 
template<typename T>
class Singleton {
    public:
        static T& GetInstance() {
            static T instance; // 这⾥要初始化DesignPattern，需要调⽤DesignPattern构造函数，同时会调⽤⽗类的构造函数
            return instance;
        }

    protected:
        virtual ~Singleton() {}
        Singleton() {}  // protected修饰构造函数，才能让别⼈继承
        Singleton(const Singleton&clone) {}
        Singleton& operator =(const Singleton&) {}

};

class DesignPattern : public Singleton<DesignPattern> {
    friend class Singleton<DesignPattern>;   // friend 能让Singleton<T> 访问到DesignPattern构造函数

    public:
    ~DesignPattern() {}
    private:
    DesignPattern() {}
    DesignPattern(const DesignPattern&) {}
    DesignPattern& operator=(const DesignPattern&) {}
};

#endif

int main() {

    return 0;
}
