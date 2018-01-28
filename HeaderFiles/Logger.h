#pragma once  

#include <string>  
#include <boost/log/trivial.hpp>  
#include <boost/log/attributes/named_scope.hpp>  

using std::string;

#define LOG_DEBUG BOOST_LOG_FUNCTION();BOOST_LOG_SEV((MyLog::s_slg), (boost::log::trivial::debug))
#define LOG_INFO BOOST_LOG_FUNCTION();BOOST_LOG_SEV((MyLog::s_slg), (boost::log::trivial::info))
//#define //LOG_ERROR BOOST_LOG_FUNCTION();BOOST_LOG_SEV((MyLog::s_slg), (boost::log::trivial::error))
#define LOG_WARNING BOOST_LOG_FUNCTION();BOOST_LOG_SEV((MyLog::s_slg), (boost::log::trivial::warning))

class MyLog
{
public:
	MyLog();
	~MyLog(void);
	
	static void Init(string  dir = "");
	static void Log(const string & msg);
	static boost::log::sources::severity_logger<boost::log::trivial::severity_level > s_slg;
protected:
private:
	static bool bInit;

};