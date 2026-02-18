/*
 * Copyright 2025 Kristóf Marussy
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
#ifndef LOGID_FEATURE_HAPTICFEEDBACK_H
#define LOGID_FEATURE_HAPTICFEEDBACK_H

#include <backend/hidpp20/features/HapticFeedback.h>
#include <features/DeviceFeature.h>
#include <ipcgull/interface.h>
#include <atomic>

namespace logid::features {
    class HapticFeedback : public DeviceFeature {
    public:
        void configure() final;

        void listen() final;

        void setProfile(config::Profile& profile) final;

        void setStrength(uint8_t strength, bool enabled = true, bool battery_saving = false);

        void playEffect(uint8_t effect);

    protected:
        explicit HapticFeedback(Device* dev);

    private:
        class IPC;

        std::atomic<bool> _enabled{true};
        std::atomic<uint8_t> _strength{60};
        std::atomic<bool> _battery_saving{false};

        std::shared_ptr<backend::hidpp20::HapticFeedback> _haptic_feedback;
        std::shared_ptr<IPC> _ipc_interface;
    };
}

#endif //LOGID_FEATURE_HAPTICFEEDBACK_H
