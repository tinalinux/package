#ifndef CPPSTRING_H
#define CPPSTRING_H

#define TI_STRING

#ifndef TI_STRING
    #include <string>
    typedef std::string CppString;
#else
    #include "tinystr.h"
    typedef TiXmlString CppString;
#endif

#endif // CPPSTRING_H
