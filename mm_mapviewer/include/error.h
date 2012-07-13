#pragma once 
class  error : public std::exception
{
    private:
        std::string     m_what;
    public:
      error( const std::string& message ):m_what(message){}
      error( const char* message ):m_what(message){}
      ~error() throw(){}
      virtual const char * what() const throw() { return m_what.c_str(); }
};

void CheckGlError( const char *str );