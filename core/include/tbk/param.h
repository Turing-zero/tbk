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
    std::mutex _mutex;
    T _value;
};
class Server:public tbk::Singleton<Server>{
    friend class etcd::ParamsServer;
public:
    Server():_etcd_params_server(etcd::ParamsServerParam(std::bind(&Server::_cb, this, std::placeholders::_1, std::placeholders::_2))){};
    ~Server() = default;
    void addCallback(const std::string& name,const std::function<void(const std::string&)>& cb){
        _callbacks[name] = cb;
    }
    void removeCallback(const std::string& name){
        _callbacks.erase(name);
    }
    template<typename T>
    void set(const std::string& name,const T& value){
        _etcd_params_server.set(name,convert<T>::from(value));
    }
    template<typename T>
    T get(const std::string& name,const T& default_value){
        auto resp = _etcd_params_server.get(name);
        if(!resp.success){
            set(name,default_value);
            return default_value;
        }
        return convert<T>::to(resp.value);
    }
    template<typename T>
    T get(const std::string& name){
        auto resp = _etcd_params_server.get(name);
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
    Param(std::string&& name,const T& default_value,const std::function<void(const T&,const T&)>& callback={}):_name(name),_callback(callback){
        param::Server::instance()->addCallback(name,std::bind(&Param::update,this,std::placeholders::_1));
        T get_value = param::Server::instance()->get<T>(name,default_value);
        if constexpr(tbk::is_atomic<T>){
            _var._value.store(get_value);
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            _var._value = get_value;
        }
    }
    Param(std::string&& name,const std::function<void(const T&,const T&)>& callback={}):_name(name),_callback(callback){
        param::Server::instance()->addCallback(name,std::bind(&Param::update,this,std::placeholders::_1));
        T get_value = param::Server::instance()->get<T>(name);
        if constexpr(tbk::is_atomic<T>){
            _var._value.store(get_value);
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            _var._value = get_value;
        }
    }
    ~Param(){
        param::Server::instance()->removeCallback(_name);
    };
    std::string name() const{
        return _name;
    }
    T get(){
        if constexpr(tbk::is_atomic<T>){
            return _var._value.load();
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            return _var._value;
        }
    }
    void set(const T& value){
        param::Server::instance()->set<T>(_name,value);
        _set(value);
    }
private:
    void _set(const T& value){
        if constexpr(tbk::is_atomic<T>){
            T prev = _var._value.load();
            _var._value.store(value);
            if(_callback){
                _callback(prev,value);
            }
        }else{
            std::scoped_lock<std::mutex> lock(_var._mutex);
            T prev = _var._value;
            _var._value = value;
            if(_callback){
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