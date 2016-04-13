#include <node.h>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace detail {
/*!
 *
 */
int unset_cloexec (int fd) {
    int flags = fcntl(fd, F_GETFD, 0);
    if (flags <  0) {
        return flags;
    }
    flags &= ~FD_CLOEXEC;
    return fcntl(fd, F_SETFD, flags);
} 


/*!
 * @function parse_arguments   Parse the input arguments, and return a vector of string.
 */
//std::vector<std::string> parse_arguments(const v8::Arguments& args) {
std::vector<std::string> parse_arguments(v8::Isolate* isolate, const v8::FunctionCallbackInfo<v8::Value>& args) {
   std::vector<std::string> str_vec;
   if(args.Length() == 1 && args[0]->IsArray()) {
      // handle array
      v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(args[0]);
      for(uint32_t i = 0; i < array->Length(); ++i) {
         if(!array->Get(i)->IsString()) {
            //v8::ThrowException(
            isolate->ThrowException(
               v8::Exception::TypeError(v8::String::Concat(v8::String::Concat(
                  v8::String::NewFromUtf8(isolate, "Argument ") ,
                  v8::Integer::New(isolate, i + 1)->ToString() ),
                  v8::String::NewFromUtf8(isolate, " must be String.")
               ))
            );
            return std::vector<std::string>();
         }
         str_vec.push_back(std::string(*v8::String::Utf8Value(array->Get(i)->ToString())));
      }
   } else {
      // handle individual args
      for(int i = 0; i < args.Length(); ++i) {
         if(!args[i]->IsString()) {
            //v8::ThrowException(
            isolate->ThrowException(
               v8::Exception::TypeError(v8::String::Concat(v8::String::Concat(
                  v8::String::NewFromUtf8(isolate, "Argument ") ,
                  v8::Integer::New(isolate, i + 1)->ToString() ),
                  v8::String::NewFromUtf8(isolate, " must be String.")
               ))
            );
            return std::vector<std::string>();
         }
         str_vec.push_back(std::string(*v8::String::Utf8Value(args[i]->ToString())));
      }
   }

   return str_vec;
}

/*!
 *
 */
std::vector<char*> to_argv(const std::vector<std::string>& cmd)                                   
{                                                                                              
   std::vector<char*> cmd_char( cmd.size() + 1 );
   for(uint32_t i = 0; i < cmd.size(); ++i) {
      cmd_char[i] = const_cast<char*>(cmd[i].c_str());
   }
   cmd_char[cmd.size()] = NULL;
   return cmd_char;                                                                            
}

/*!
 *
 */
void handle_error(int status, const std::string& str) {
   if(status == -1) {
      perror(std::string(str + ": ").c_str());
   }
}

} /* namespace detail */

/*!
 * @function execvp
 */
//v8::Handle<v8::Value> Execvp(const v8::Arguments& args) {
void Execvp(const v8::FunctionCallbackInfo<v8::Value>& args) {
   //v8::HandleScope scope;
   v8::Isolate* isolate = args.GetIsolate();
   
   // check input
   std::vector<std::string> str_vec = detail::parse_arguments(isolate, args);
   if(str_vec.size() == 0) {
      //return scope.Close(v8::Integer::New(-1));
      args.GetReturnValue().Set(v8::Integer::New(isolate, -1));
      return;
   }
   std::vector<char*> argv = detail::to_argv(str_vec);

   // get ready for the exec
   int status;
   detail::unset_cloexec(0);
   detail::unset_cloexec(1);
   detail::unset_cloexec(2);
   detail::handle_error(status = execvp(argv[0], &argv[0]), "execvp");
   
   //return scope.Close(v8::Integer::New(status));
   args.GetReturnValue().Set(v8::Integer::New(isolate, status));
}

/*!
 *
 */
//v8::Handle<v8::Value> Chroot(const v8::Arguments& args) {
void Chroot(const v8::FunctionCallbackInfo<v8::Value>& args) {
   //v8::HandleScope scope;
   v8::Isolate* isolate = args.GetIsolate();

   // check input
   std::vector<std::string> str_vec = detail::parse_arguments(isolate, args);
   if(str_vec.size() != 1) {
      //return scope.Close(v8::Integer::New(isolate, -1));
      args.GetReturnValue().Set(v8::Integer::New(isolate, -1));
   }
   std::vector<char*> argv = detail::to_argv(str_vec);

   int status;
   detail::handle_error(status = chroot(argv[0]), "chroot");

   //return scope.Close(v8::Integer::New(status));
   args.GetReturnValue().Set(v8::Integer::New(isolate, status));
}

/*!
 *
 */
//v8::Handle<v8::Value> Chdir(const v8::Arguments& args) {
void Chdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
   //v8::HandleScope scope;
   v8::Isolate* isolate = args.GetIsolate();

   // check input
   std::vector<std::string> str_vec = detail::parse_arguments(isolate, args);
   if(str_vec.size() != 1) {
      //return scope.Close(v8::Integer::New(isolate, -1));
      args.GetReturnValue().Set(v8::Integer::New(isolate, -1));
   }
   std::vector<char*> argv = detail::to_argv(str_vec);

   int status;
   detail::handle_error(status = chdir(argv[0]), "chdir");

   //return scope.Close(v8::Integer::New(status));
   args.GetReturnValue().Set(v8::Integer::New(isolate, status));
}

/*!
 * Register all module interface functions
 */
//void Init(v8::Handle<v8::Object> target) {
void Init(v8::Local<v8::Object> exports) { //, v8::Local<v8::Object> module) {
   // execvp
   NODE_SET_METHOD(exports, "execvp", Execvp);
   NODE_SET_METHOD(exports, "chroot", Chroot);
   NODE_SET_METHOD(exports, "chdir" , Chdir);
}

// Register the module with node. Note that "modulename" must be the same as
// the basename of the resulting .node file. You can specify that name in
// binding.gyp ("target_name"). When you change it there, change it here too.
NODE_MODULE(easyexec, Init);
