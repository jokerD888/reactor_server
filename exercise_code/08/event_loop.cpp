#include "event_loop.h"

void EventLoop::Run() {
    while (true) {
        std::vector<Channel*> channels = ep_->Loop();
        for (auto ch : channels) ch->HandleEvent();
    }
}