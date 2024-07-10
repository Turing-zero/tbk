#ifndef __TBK_PARAM_H__
#define __TBK_PARAM_H__
#include "tbk/meta.h"
#include "tbk/utils/singleton.h"
#include "tbk/process/etcdif.h"
#include "tbk/paramconvert.h"
namespace tbk{
namespace param{
template<typename T, bool flag>
struct Var{};
// use atomic if supported
template<typename T>
struct Var<T,true>{
    std::atomic<T> _value;
};
// use mutex if atomic not supported
template<typename T>
struct Var<T,false>{
    mutable std::mutex _mutex;
    T _value;
};
class Server:public tbk::Singleton<Server>{
    inline const static std::string _VALUE_PREFIX = "/__v__";
    inline const static std::string _INFO_PREFIX = "/__i__";
    inline const static std::string _TYPE_PREFIX = "/__t__";
    friend class etcd::ParamsServer;
public:
    Server():_etcd_params_server(etcd::ParamsServerParam(std::bind(&Server::_cb, this, std::placeholders::_1, std::placeholders::_2))){};
    ~Server() = default;
    void addCallback(const std::string& name,const std::function<void(const std::string&)>& cb){
        _callbacks[name+_VALUE_PREFIX] = cb;
    }
    void removeCallback(const std::string& name){
        _callbacks.erase(name);
    }
    template<typename T>
    void set(const std::string& name,const T& value){
        _etcd_params_server.set(name+_VALUE_PREFIX,convert<T>::from(value));
        _etcd_params_server.set(name+_TYPE_PREFIX,convert<T>::type());
        _etcd_params_server.set(name+_INFO_PREFIX,convert<T>::info());
    }
    template<typename T>
    T get(const std::string& name,const T& default_value){
        auto resp = _etcd_params_server.get(name+_VALUE_PREFIX);
        if(!resp.success){
            set(name,default_value);
            return default_value;
        }
        return convert<T>::to(resp.value);
    }
    template<typename T>
    T get(const std::string& name){
        auto resp = _etcd_params_server.get(name+_VALUE_PREFIX);
        if(!resp.success){
            tbk::error("param {} not exist\n",name);
            return T();
        }
        return convert<T>::to(resp.value);
    }
private:
    void _cb(const std::string& key,const std::string& value){
        tbk::info("in server cb key: {}, value: {}\n",key,value);
        auto it = _callbacks.find(key);
        if(it != _callbacks.end()){
            it->second(value);
        }
    }
    etcd::ParamsServer _etcd_params_server;
    std::unordered_map<std::string,std::function<void(const std::string&)>> _callbacks;
};
} // namespace param

template<typename T>
class Param{
    friend class param::Server;
public:
    Param(const std::string& name,const T& default_value,const std::function<void(const T&,const T&)>& callback={},const bool immediate_call=false):_name(name),_callback(callback){
        param::Server::instance()->addCallback(name,std::bind(&Param::update,this,std::placeholders::_1));
        _set(default_value,immediate_call);
        T get_value = param::Server::instance()->get<T>(name,default_value);
        tbk::log("testtest - {}\n",param::convert<T>::from(get_value));
        _set(get_value,immediate_call);
    }
    Param(const std::string& name,const std::function<void(const T&,const T&)>& callback={}):Param(name,T(),callback){
    }
    ~Param(){
        param::Server::instance()->removeCallback(_name);
    };
    std::string name() const{
        return _name;
    }
    T get() const{
        if constexpr(tbk::is_atomic<T>){
            return _var._value.load();
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            return _var._value;
        }
    }
    void set(const T& value){
        _set(value);
        param::Server::instance()->set<T>(_name,value);
    }
private:
    void _set(const T& value,const bool with_callback=true){
        if constexpr(tbk::is_atomic<T>){
            T prev = _var._value.load();
            if(prev == value)
                return;
            _var._value.store(value);
            if(with_callback && _callback){
                _callback(prev,value);
            }
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            T prev = _var._value;
            if(prev == value)
                return;
            _var._value = value;
            if(with_callback &&_callback){
                _callback(prev,value);
            }
        }
    }
    void update(const std::string& value){
        _set(param::convert<T>::to(value));
    }
    std::string _name;
    param::Var<T,tbk::is_atomic<T>> _var;
    std::function<void(const T&,const T&)> _callback = {};
};
namespace param{
using Int = Param<int>;
using Double = Param<double>;
using String = Param<std::string>;
using Bool = Param<bool>;
} // namespace param
} // namespace tbk
#endif // __TBK_PARAM_H__ 