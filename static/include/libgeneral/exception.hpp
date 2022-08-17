//
//  exception.hpp
//  libgeneral
//
//  Created by tihmstar on 09.03.18.
//  Copyright © 2018 tihmstar. All rights reserved.
//

#ifndef exception_hpp
#define exception_hpp

#include <string>
#include <stdarg.h>

namespace tihmstar {
    class exception : public std::exception{
        const char *_commit_count_str;
        const char *_commit_sha_str;
        int _line;
        std::string _filename;
        char *_err;
        
    public:
        exception(const char *commit_count_str, const char *commit_sha_str, int line, const char *filename, const char *err ...);
        exception(const char *commit_count_str, const char *commit_sha_str, int line, const char *filename, const char *err, va_list ap);

        exception(const exception &e); //copy constructor
        
        //custom error can be used
        virtual const char *what() const noexcept override;
        
        /*
         -first lowest two bytes of code is sourcecode line
         -next two bytes is strlen of filename in which error happened
         */
        int code() const;
        
        virtual void dump() const;
        virtual std::string dumpStr() const;

        //Information about build
        virtual std::string build_commit_count() const;
        virtual std::string build_commit_sha() const;
        
        ~exception();
    };
};

#endif /* exception_hpp */
