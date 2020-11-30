#pragma once
#ifndef PCH
    #include <chrono>
    #include <memory>
    #include <string_view>
    #include <vector>
#endif

namespace download
{
    enum class protocol_t: std::uint8_t
    {
        http,
        https
    };

    class task
    {
    public:
        using milliseconds = std::chrono::milliseconds;

        task() = default;

        task(std::string host, std::string resource, protocol_t protocol = protocol_t::https):
            _host(std::move(host)),
            _resource(std::move(resource)),
            _protocol(protocol)
        {
        }

        virtual ~task() = default;

        const std::string& host() const noexcept { return _host; }
        void set_host(std::string item) { _host = std::move(item); }

        const std::string& resource() const noexcept { return _resource; }
        void set_resource(std::string item) { _resource = std::move(item); }

        protocol_t protocol() const noexcept { return _protocol; }
        void set_protocol(protocol_t item) noexcept { _protocol = item; }

        const milliseconds& duration() const noexcept { return _duration; }
        void set_duration(const milliseconds& item) noexcept { _duration = item; }

        const std::shared_ptr<std::exception>& exception() const { return _exception; }
        void set_exception(std::shared_ptr<std::exception> item) { _exception = std::move(item); }
        void set_exception(std::string message) { _exception = std::make_shared<std::runtime_error>(std::move(message)); }

        virtual void set_data(const std::string_view& content_type, const std::string& received_data) = 0;

    protected:
        std::string _host;
        std::string _resource;
        std::shared_ptr<std::exception> _exception {};
        std::chrono::milliseconds _duration {};
        protocol_t _protocol = protocol_t::https;
    };
}
