/*
 * Copyright 2026 Brad Fullwood
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <features/ScrollSmoother.h>
#include <ipc_defs.h>
#include <util/task.h>

using namespace logid::features;

class ScrollSmoother::IPC : public ipcgull::interface {
public:
    explicit IPC(ScrollSmoother* parent) : ipcgull::interface(
            SERVICE_ROOT_NAME ".ScrollSmoother", {
                    {"GetSmoothScroll",   {this, &IPC::get,         {"interval", "steps"}}},
                    {"SetScrollInterval", {this, &IPC::setInterval, {"interval"}}},
                    {"SetScrollSteps",    {this, &IPC::setSteps,    {"steps"}}},
            }, {}, {}), _parent(*parent) {
    }

    std::tuple<int, int> get() const {
        return {_parent.intervalMs(), _parent.steps()};
    }

    void setInterval(int interval) {
        _parent.setIntervalMs(interval);
    }

    void setSteps(int steps) {
        _parent.setSteps(steps);
    }

private:
    ScrollSmoother& _parent;
};

ScrollSmoother::ScrollSmoother(std::shared_ptr<ipcgull::node> node) {
    if (node) {
        _ipc = node->make_interface<IPC>(this);
    }
}

void ScrollSmoother::feed(int32_t deltaV) {
    _buffer += deltaV;
    if (!_draining.exchange(true)) {
        run_task([weak = weak_from_this()]() {
            if (auto self = weak.lock())
                self->_drain();
        });
    }
}

void ScrollSmoother::reset() {
    _buffer = 0;
}

void ScrollSmoother::setOnChunk(ChunkCallback cb) {
    _on_chunk = std::move(cb);
}

int32_t ScrollSmoother::_drainOnce() {
    int32_t buf = _buffer.load();
    if (buf == 0) {
        _draining = false;
        return 0;
    }

    int32_t abs_buf = (buf > 0) ? buf : -buf;
    int s = _steps.load();
    int32_t abs_chunk = abs_buf / s;
    if (abs_chunk < 1) abs_chunk = 1;

    if (buf > 0) {
        _buffer -= abs_chunk;
        return abs_chunk;
    } else {
        _buffer += abs_chunk;
        return -abs_chunk;
    }
}

void ScrollSmoother::_drain() {
    int32_t chunk = _drainOnce();
    if (chunk != 0 && _on_chunk)
        _on_chunk(chunk);

    if (_buffer.load() != 0) {
        run_task_after([weak = weak_from_this()]() {
            if (auto self = weak.lock())
                self->_drain();
        }, std::chrono::milliseconds(_interval_ms.load()));
    } else {
        _draining = false;
    }
}

int ScrollSmoother::intervalMs() const {
    return _interval_ms.load();
}

void ScrollSmoother::setIntervalMs(int ms) {
    if (ms < 1) ms = 1;
    _interval_ms = ms;
}

int ScrollSmoother::steps() const {
    return _steps.load();
}

void ScrollSmoother::setSteps(int steps) {
    if (steps < 1) steps = 1;
    _steps = steps;
}
