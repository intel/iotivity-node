#include <node.h>
#include <v8.h>

using namespace v8;

Handle<Value> get_hello(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("world"));
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("hello"),
      FunctionTemplate::New(get_hello)->GetFunction());
}

NODE_MODULE(iotivity, Init)
