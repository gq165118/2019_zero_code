#include <string> 
#if 0
// 实现导出数据的接口, 导出数据的格式包含 xml，json，文本格式txt 后面可能扩展excel格式csv
class IExport {
public:
    virtual bool Export(const std::string& data) = 0;
    virtual ~IExport() {}
}；

class ExportXml : public IExport {
public:
    virtual bool Export(const std::string& data) {
        return true;
    } 
};

class ExportJson : public IExport {
public:
    virtual bool Export(const std::string& data) {
        return true;
    } 
};

class ExportTxt : public IExport {
public:
    virtual bool Export(const std::string& data) {
        return true;
    } 
};

int main() {
    std::string choose 
    if (choose == "txt") {  //具体的实现细节
        /*....*/
        IExport *e = new ExportTxt();
        e->Export("hello world");
        /*....*/
    }else if (choose == "json") {
        /*....*/
        IExport *e = new ExportJson();
        e->Export("hello world");
        /*....*/
    } else if (choose == "xml") {
        /*....*/
        IExport *e = new ExportXml();
        e->Export("hello world");
        /*....*/
    }
}

#elif 0  //屏蔽每个格式的实现细节 
//符合设计原则的地方保留
class IExport {
public:
    virtual bool Export(const std::string &data) = 0;
    virtual ~IExport(){}
};

class ExportXml : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class ExportJson : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class ExportTxt : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

//封装导出数据具体的实现细节
class IExportFactory {  
public:
    virtual IExport* NewExport(/*.....*/) = 0;
};

class ExportXmlFactory : IExportFactory {
public:
    IExport* NewExport(/*...*/) {
        // 可能有其它操作，或者许多参数
        IExport* temp = new ExportXml
        return temp;
    }
};

class ExportJsonFactory : IExportFactory {
public:
    IExport* NewExport(/*...*/) {
        // 可能有其它操作，或者许多参数
        IExport* temp = new ExportXml
        return temp;
    }
};

class ExportTxtFactory : IExportFactory {
public:
    IExport* NewExport(/*...*/) {
        // 可能有其它操作，或者许多参数
        IExport* temp = new ExportXml
        return temp;
    }
};

//利用组合的方式(组合基类的指针)封装了导出数据细节
class ExportData {
public:
    ExportData(IExportFactory* factory) : _factory(factory) {}
    ~ExportData() {
        if (_factory) {
            delete _/factory;
            _factory = nullptr;
        }
    }
    //稳定的流畅往基类放
    bool Export(const std::string& data) {  //还可以继续优化，放在IExportFactory类中
        IExport* e = _factory->NewExport();
        e->Export(data);
    }

private:
    IExportFactory* _factory;
};

int mian() {
    ExportData ed(new ExportTxtFactory);
    ed.Export("hello word");
    return 0;
}


#else 

class IExport {
public:
    virtual bool Export(const std::string &data) = 0;
    virtual ~IExport(){}
};

class ExportXml : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class ExportJson : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class ExportTxt : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class ExportCSV : public IExport {
public:
    virtual bool Export(const std::string &data) {
        return true;
    }
};

class IExportFactory {
public:
    IExportFactory() {
        _export = nullptr;
    }

    virtual ~IExportFactory() {
        if (_export) {
            delete _export;
            _export = nullptr;
        }
    }

    bool Export(const std::striung& data) {
        if (_export = nullptr) {
            _export = NewExport();
        }
        return _export->Export(data);
    }

protected:
    virtual IExport* NewExport(/*...*/) = 0;

private:
    IExport* _export;
};

class ExportXmlFactory : public IExportFactory {
protected:
    virtual IExport * NewExport(/* ... */) {
        // 可能有其它操作，或者许多参数
        IExport * temp = new ExportXml();
        // 可能之后有什么操作
        return temp;
    }
};
class ExportJsonFactory : public IExportFactory {
protected:
    virtual IExport * NewExport(/* ... */) {
        // 可能有其它操作，或者许多参数
        IExport * temp = new ExportJson;
        // 可能之后有什么操作
        return temp;
    }
};
class ExportTxtFactory : public IExportFactory {
protected:
    IExport * NewExport(/* ... */) {
        // 可能有其它操作，或者许多参数
        IExport * temp = new ExportTxt;
        // 可能之后有什么操作
        return temp;
    }
};


int main() {
    IExportFactory *factory = new ExportTxtFactory();
    factory->Export("hello world");
    return 0;
}

#endif
