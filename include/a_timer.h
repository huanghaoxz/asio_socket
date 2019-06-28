//
// Created by huanghao on 19-5-28.
//

#ifndef HBAUDITFLOW_A_TIMER_H
#define HBAUDITFLOW_A_TIMER_H

#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <functional>
using namespace std;
using namespace std::placeholders;
using namespace boost;
using namespace boost::asio;


class a_timer {
private:
    boost::function<void()> f;//function 对象,持有无参无返回值的可调对象
    boost::asio::deadline_timer t;//asio 定时器对象
public:
    template <typename F>
    a_timer(boost::asio::io_service& ios,F func):f(func),t(ios,posix_time::millisec(1000))
    {
        t.async_wait(std::bind(&a_timer::call_func,this,std::placeholders::_1));
    }
    void call_func(const boost::system::error_code& err)
    {
        if(!err)
        {
            f();
            t.expires_at(t.expires_at()+posix_time::millisec(1000));
            t.async_wait(std::bind(&a_timer::call_func,this,std::placeholders::_1));
        }
    }
};


#endif //HBAUDITFLOW_A_TIMER_H
