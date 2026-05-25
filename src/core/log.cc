#include <unigui/core/log.h>

namespace unigui {

void InitLogging(const std::string& level) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("unigui.log", true);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    auto logger = std::make_shared<spdlog::logger>("unigui", spdlog::sinks_init_list{console_sink, file_sink});
    auto lvl = spdlog::level::from_str(level);
    logger->set_level(lvl);
    logger->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(logger);
}

std::shared_ptr<spdlog::logger> GetLogger() {
    auto l = spdlog::get("unigui");
    if (!l) l = spdlog::default_logger();
    if (!l) { l = spdlog::stdout_color_mt("unigui_fallback"); }
    return l;
}

} // namespace unigui
