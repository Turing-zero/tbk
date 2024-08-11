#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
// #include <pybind11/stl.h>
#include "tbk/tbk.h"

int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;
using py_callback_type = std::function<void(const py::bytes&)>;

class SubscriberPy: public tbk::Subscriber<0>{
    // https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
    // don't understand why but need to be fixed
    py_callback_type _py_callback = {};
    void _cpp_cb(const tbk::Data& data){
        if(_py_callback){
            py::gil_scoped_acquire guard{};
            _py_callback(py::bytes(static_cast<const char*>(data.data()), data.size()));
        }
    }
public:
    SubscriberPy(const std::string& name, const std::string& msg_name, const py_callback_type& f = {}):SubscriberPy(3, name, msg_name, f){}
    SubscriberPy(const int buffer_size, const std::string& name, const std::string& msg_name, const py_callback_type& f = {}):tbk::Subscriber<0>(buffer_size, name, msg_name, std::bind(&SubscriberPy::_cpp_cb,this,std::placeholders::_1)),_py_callback(f){}
    std::string test(){
        return "test";
    }
};
PYBIND11_MODULE(tbkpylib, m) {
    m.doc() = "tbkpylib module";
    m.def("init",&tbk::init, "tbk init function", py::arg("name")="", py::arg("symbol")="");
    py::class_<tbk::Data>(m, "Data")
        .def(py::init<>());
    py::class_<SubscriberPy>(m, "Subscriber")
        .def(py::init<const int,const std::string&, const std::string&, const py_callback_type&>(), py::arg("buffer_size"), py::arg("name"), py::arg("msg_name"), py::arg("f")=py_callback_type())
        .def(py::init<const std::string&, const std::string&, const py_callback_type&>(), py::arg("name"), py::arg("msg_name"), py::arg("f")=tbk::__callback_type())
        .def("test", &SubscriberPy::test);
    py::class_<tbk::Publisher>(m, "Publisher")
        .def(py::init<const std::string&, const std::string&>(), py::arg("name"), py::arg("msg_name"))
        .def("publish", py::overload_cast<const std::string &>(&tbk::Publisher::publish), py::arg("data"));
    m.def("add", &add, "A function which adds two numbers");
}