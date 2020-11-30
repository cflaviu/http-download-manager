#pragma once
#ifndef PCH
    #include <download/agent/base.hpp>
#endif

namespace download
{
    namespace agent
    {
        class http: public base<boost::asio::ip::tcp::socket>
        {
        public:
            using parent_t = base<boost::asio::ip::tcp::socket>;

            http(size_t data_max_size = 64 * 1024, size_t buffer_max_size = 4 * 1024):
                parent_t(data_max_size, buffer_max_size)
            {
                _socket = std::make_unique<boost::asio::ip::tcp::socket>(_context);
            }

            void operator()(task& my_task)
            {
                connect(my_task);
                execute(my_task);
            }

        protected:
            void connect(task& my_task)
            {
                set_start_time();
                auto endpoints = _resolver.resolve(my_task.host(), "http");
                boost::asio::connect(*_socket, endpoints);
            }
        };
    }
}
