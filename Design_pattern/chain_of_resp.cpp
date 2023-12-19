#include <string> 


#if 0 //不用责任链方式写
class Context {
public:
    std::string name;
    int day;
};

class LeaveRequest {
public:
    bool HandleRequest(const Context& ctx) {
        if (ctx.day <= 3)
                HandleByMainProgram(ctx);
        else if (ctx.day <= 10) 
            HandleByProjMgr(ctx);
        else
            HandleByBoss(ctx);
    }

private:
    bool HandleByMainProgram(const Context& ctx) {

    }

    bool HandleByProjMgr(const Context& ctx) {
        
    }

    bool HandleByBoss(const Context& ctx) {
        
    }

};



#else 
class Context {
public:
    std::string name;
    int day;
};

class IHandler {
public:
    virtual ~IHandler() {}
    void SetNextHandler(IHandler* next){
        next = next;
    }

    bool Handle(const Context& ctx) {
        if (CanHandle(ctx)){
            return HandleRequest(ctx);
        }else if (GetNextHandler()) {
            return GetNextHandler()->HandleRequest(ctx);
        }else{
            //err
        }
        return false;
    }

protected:
    virtual bool HandleRequest(const Context& ctx) = 0;
    virtual bool CanHandle(const Context& ctx) = 0;
    IHandler* GetNextHandler() {
        return next;
    }

private:
    IHandler* next;
};

class HanleByMainProgram : public IHandler {
protected:
    virtual bool HandleRequest(const Context& ctx){
        //
        return true;
    }

    virtual bool CanHandle(const Context& ctx) {
        //
        return true;
    }

};

class HandleByProjMgr : public IHandler {
protected:
    virtual bool HandleRequest(const Context &ctx){
        //
        return true;
    }
    virtual bool CanHandle(const Context &ctx) {
        //
        return true;
    }
};

class HandleByBoss : public IHandler {
protected:
    virtual bool HandleRequest(const Context &ctx){
        //
        return true;
    }
    virtual bool CanHandle(const Context &ctx) {
        //
        return true;
    }
};

class HandleByBeauty : public IHandler {
protected:
    virtual bool HandleRequest(const Context &ctx){
        //
        return true;
    }
    virtual bool CanHandle(const Context &ctx) {
        //
        return true;
    }
};

class ProcessOfReq {
public:
    void NewChainOfReq () {
        IHandler* h0 = new HandleByBeauty();
        IHandler* h1 = new HandleByMainProgram();
        IHandler* h2 = new HandleByProjMgr();
        IHandler* h3 = new HandleByBoss();

        h0->SetNextHandler(h1);
        h1->SetNextHandler(h2);
        h2->SetNextHandler(h3);
    }
};


int mian() {

    ProcessOfReq req1;
    q1.NewChainOfReq();

    //设置下一指针
    Context ctx;
    h0->Handle(ctx);

    return 0
}

#endif



