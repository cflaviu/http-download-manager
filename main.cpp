#include <download/manager.hpp>
#include <iostream>
#include <unordered_set>
#include <future>

namespace download
{
    class rss_task: public task
    {
    public:
        using array = std::vector<rss_task>;
        using notifier_t = std::function<void(const rss_task&)>;

        using task::task;

        const std::string& data() const noexcept { return _data; }

        const notifier_t& notifier() const noexcept { return _notifier; }
        void set_notifier(notifier_t item) { _notifier = std::move(item); }

    protected:
        void set_data(const std::string_view& content_type, const std::string& received_data) override
        {
            static const std::unordered_set<std::string_view> accepted_content_type
            {
                "application/rss+xml",
                "application/xml",
                "text/html",
                "text/xml",
            };

            if (accepted_content_type.contains(content_type))
            {
                _data = std::move(received_data);
            }
            else
            {
                std::string message;
                if (!content_type.empty())
                {
                    message = "unrecognized content type ";
                    message += content_type;
                }
                else
                {
                    message = "content type not specified";
                }

                set_exception(std::move(message));
            }

            if (_notifier)
            {
                static_cast<void>(std::async(std::launch::async, [this]()
                {
                    try
                    {
                        _notifier(*this);
                    }
                    catch(...)
                    {}
                }));
            }
        }

        std::string _data;
        notifier_t _notifier;
    };
}

int main()
{
    using namespace download;

    rss_task::array tasks
    {
        { "www.news.ro", "/rss" },
        { "www.profit.ro", "/rss" },
        { "rss.slashdot.org", "/slashdot/slashdot", protocol_t::http },
    };

    for(auto& task : tasks)
    {
        task.set_notifier([](const rss_task& /*task*/) { std::cout << "task done\n"; });
    }

    download::manager manager(2, 256 * 1024);
    manager.execute(tasks);

    std::cout << '\n';
    for(auto& task : tasks)
    {
        std::cout << task.host() << task.resource() << ":\n";
        if (task.exception())
        {
            std::cout << "error: " << task.exception()->what() << "\n\n";
        }
        else
        {
            std::cout << "time: " << task.duration().count() << " ms\ndata: " << task.data().size() << " B\n\n";
        }
    }

    return 0;
}
