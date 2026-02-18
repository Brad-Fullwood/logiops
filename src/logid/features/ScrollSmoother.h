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
#ifndef LOGID_FEATURE_SCROLLSMOOTHER_H
#define LOGID_FEATURE_SCROLLSMOOTHER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <ipcgull/interface.h>
#include <ipcgull/node.h>

namespace logid::features {

    class ScrollSmoother : public std::enable_shared_from_this<ScrollSmoother> {
    public:
        using ChunkCallback = std::function<void(int32_t)>;

        explicit ScrollSmoother(std::shared_ptr<ipcgull::node> node);

        void feed(int32_t deltaV);
        void reset();

        void setOnChunk(ChunkCallback cb);

        int intervalMs() const;
        void setIntervalMs(int ms);
        int steps() const;
        void setSteps(int steps);

    private:
        void _drain();
        int32_t _drainOnce();

        std::atomic<int32_t> _buffer{0};
        std::atomic<bool> _draining{false};
        std::atomic<int> _interval_ms{8};
        std::atomic<int> _steps{10};
        ChunkCallback _on_chunk;

        class IPC;
        std::shared_ptr<IPC> _ipc;
    };

}

#endif //LOGID_FEATURE_SCROLLSMOOTHER_H
