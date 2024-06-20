//#include <iostream>
namespace pattern
{
    namespace OldNamespace
    {
        
    }
}

class CArray
{
    public:
    void Add(){}
};

int F(int x);

int h(int x) { return F(x); }

int loop()
{

    //proto::ProtoCommandLineFlag flag;
    //int x = flag.foo();
    //int y = flag.GetProto().foo();

    h(1);

    testFun();
    TestFun2();

    CArray array;
    array.Add();
    if (0)
    {
        for (int i = 0; i < 10; i++)
        {

        }
        int j = 1;
    }
    return 0;
}