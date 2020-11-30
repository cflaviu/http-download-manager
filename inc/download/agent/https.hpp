#pragma once
#ifndef PCH
    #include <boost/asio/ssl.hpp>
    #include <download/agent/base.hpp>
#endif

namespace download
{
    namespace agent
    {
        class https: public base<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>
        {
        public:
            using parent_t = base<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

            https(size_t data_max_size = 64 * 1024, size_t buffer_max_size = 4 * 1024):
                parent_t(data_max_size, buffer_max_size),
                _tls_context(boost::asio::ssl::context::tls)
            {
                _socket = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(_context, _tls_context);
            }

            void operator()(task& my_task)
            {
                connect(my_task);
                execute(my_task);
            }

        protected:
            void connect(task& my_task)
            {
                if (!SSL_set_tlsext_host_name(_socket->native_handle(), my_task.host().c_str()))
                {
                    boost::system::error_code ec(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
                    throw boost::system::system_error(ec);
                }

                set_start_time();
                auto endpoints = _resolver.resolve(my_task.host(), "https");
                boost::asio::connect(_socket->next_layer(), endpoints);
                _socket->handshake(boost::asio::ssl::stream_base::client);
            }

            boost::asio::ssl::context _tls_context;
        };
    }
}
