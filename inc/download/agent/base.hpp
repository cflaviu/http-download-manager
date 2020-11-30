#pragma once
#ifndef PCH
    #include <boost/asio/streambuf.hpp>
    #include <boost/beast/http.hpp>
    #include <boost/beast/http/parser.hpp>
    #include <boost/beast/core.hpp>
    #include <download/task.hpp>
#endif

namespace download
{
    namespace agent
    {
        template <typename Socket>
        class base
        {
        public:
            base(size_t data_max_size, size_t buffer_max_size):
                _resolver(_context),
                _buffer(buffer_max_size),
                _data_max_size(data_max_size)
            {}

        protected:
            void execute(task& my_task)
            {
                send_request(my_task);
                receive_response(my_task);
                close();
            }

            void set_start_time()
            {
                _start_time = std::chrono::high_resolution_clock::now();
            }

            void send_request(task& my_task)
            {
                boost::asio::streambuf request;
                std::ostream stream(&request);
                stream  << "GET " << my_task.resource() << " HTTP/1.1\r\n"
                        "Host: " << my_task.host() << "\r\n"
                        "User-Agent: Wget/1.20.3 (linux-gnu)\r\n"
                        "Accept: */*\r\n"
                        "Connection: close\r\n\r\n";

                boost::asio::write(*_socket, request);
            }

            static std::string_view filter_content_type(const boost::beast::string_view& content_type)
            {
                auto pos = content_type.find_first_of(';', 8);
                return {content_type.data(), (pos == std::string::npos) ? content_type.size() : pos};
            }

            void receive_response(task& my_task)
            {
                using namespace boost::beast;
                http::response_parser<http::string_body> parser(std::move(_response));
                parser.eager(true);
                parser.body_limit(_data_max_size);
                http::read(*_socket, _buffer, parser);
                _response = parser.release();
                my_task.set_duration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _start_time));
                const auto& content_type = _response[http::field::content_type];
                const auto& body = _response.body();
                my_task.set_data(filter_content_type(content_type), body);
            }

            void close()
            {
                auto& lowest_layer = _socket->lowest_layer();
                lowest_layer.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                lowest_layer.close();
            }

            boost::asio::io_context _context;
            std::unique_ptr<Socket> _socket;
            boost::asio::ip::tcp::resolver _resolver;

            boost::beast::flat_buffer _buffer;
            boost::beast::http::response<boost::beast::http::string_body> _response;
            std::chrono::high_resolution_clock::time_point _start_time;
            size_t _data_max_size;
        };
    }
}
