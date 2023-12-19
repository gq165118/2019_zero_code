// 普通员工有销售奖金，累计奖金，部门经理除此之外还有团队奖金；后面可能会添加环比增长奖金，同时可能产生不同的奖金组合；
// 销售奖金 = 当月销售额 * 4%
// 累计奖金 = 总的回款额 * 0.2%
// 部门奖金 = 团队销售额 * 1%
// 环比奖金 = (当月销售额-上月销售额) * 1%
// 销售后面的参数可能会调整

#include <iostream>

class Context {
public:
    bool isMgr;
    //User user;
    //double groupasale;
};

#if 0  //不用装饰器模式

class Bonus {
public:
    double CalcBonus(Context& ctx) {
        double bonus = 0.0;
        bonus += CalcMonthBonus(ctx);
        bonus += CalcSumBonus(ctx);
        if (ctx.isMgr) {
            bonus += CalcGroupBonus(ctx);
        }
        return bonus;
    }

private:
    double CalcMonthBonus(Context& ctx) {
        double bonus /*..计算方式。。*/;
        return bonus;
    }

    double CalcSumBonus(Context& ctx) {
        double bonus/* = */;
        return bonus;
    }
    double CalcGroupBonus(Context &ctx) {
        double bonus/* = */;
        return bonus;
    }
};

#else 
// 试着从职责出发，将职责抽象出来
class CalcBonus {
public:
    CalcBonus(CalcBonus* c = nullptr) : cc(c) {}
    virtual double Calc(Context& ctx) {
        return 0.0; // 基本工资
    }

protected:
    CalcBonus* cc;  //组合自身
};

class CalcMonthBonus : public CalcBonus {
public:
    CalcMonthBonus(CalcBonus* c) : CalcBonus(c) {}
    virtual double Calc(Context& ctx) {
        double mbonus /* = 计算流程忽略*/;
        return mbonus + cc->Calc(ctx);  //加上基类(基本工资)
    }
};

class CalcSumBonus : public CalcBonus {
public:
    CalcSumBonus(CalcBonus* c) : CalcBonus(c) {}
    virtual double Calc(Context& ctx) {
        double mbonus /* = 计算流程忽略*/;
        return mbonus + cc->Calc(ctx);  //加上基类(基本工资)
    }
};

class CalcGroupBonus : public CalcBonus {
public:
    CalcGroupBonus(CalcBonus* c) : CalcBonus(c) {}
    virtual double Calc(Context& ctx) {
        double mbonus /* = 计算流程忽略*/;
        return mbonus + cc->Calc(ctx);  //加上基类(基本工资)
    }
};

class CalcCycleBonus : public CalcBonus {
public:
    CalcCycleBonus(CalcBonus* c) : CalcBonus(c) {}
    virtual double Calc(Context& ctx) {
        double mbonus /* = 计算流程忽略*/;
        return mbonus + cc->Calc(ctx);  //加上基类(基本工资)
    }
};

#endif

int main() {
    //1. 普通员工
    Context ctx1;
    CalcBonus* base = new CalcBonus();
    CalcBonus* cb1 = new CalcSumBonus(base);
    CalcBonus* cb2 = new CalcMonthBonus(cb1);
    cb2->Calc(ctx1);

    //2. 部门经理
    Context ctx2;
    CalcBonus* cb3 = new CalcGroupBonus(cb2);
    cb3->Calc(ctx2);

    return 0;
}

