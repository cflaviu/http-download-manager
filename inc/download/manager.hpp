#pragma once
#ifndef PCH
    #include <download/agent/http.hpp>
    #include <download/agent/https.hpp>
    #include <boost/asio/thread_pool.hpp>
#endif

namespace download
{
    class manager
    {
    public:
        manager(const uint8_t thread_pool_size, size_t data_max_size, size_t buffer_max_size = 4096):
            _thread_pool(thread_pool_size),
            _data_max_size(data_max_size),
            _buffer_max_size(buffer_max_size)
        {}

        size_t data_max_size() const noexcept { return _data_max_size; }
        void set_data_max_size(size_t value) { _data_max_size = value; }

        size_t buffer_max_size() const noexcept { return _buffer_max_size; }
        void set_buffer_max_size(size_t value) { _buffer_max_size = value; }

        template <typename TaskArray>
        void execute(TaskArray& tasks)
        {
            for(auto& task : tasks)
            {
                boost::asio::post(_thread_pool, (task.protocol() == protocol_t::https) ?
                                      std::function<void()>([&]() { download<agent::https>(task); }) :
                                      std::function<void()>([&]() { download<agent::http>(task);  }));
            }

            _thread_pool.join();
        }

    protected:
        template <typename Agent>
        void download(task& my_task) noexcept
        {
            try
            {
                Agent downloader(_data_max_size, _buffer_max_size);
                downloader(my_task);
            }
            catch (std::exception& e)
            {
                my_task.set_exception(std::make_unique<std::runtime_error>(std::string(e.what())));
            }
            catch(...)
            {
                my_task.set_exception(std::make_unique<std::runtime_error>(std::string("unknown error")));
            }
        }

        boost::asio::thread_pool _thread_pool;
        size_t _data_max_size;
        size_t _buffer_max_size;
    };
}
