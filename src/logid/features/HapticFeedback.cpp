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
#include <features/HapticFeedback.h>
#include <Device.h>
#include <ipc_defs.h>
#include <algorithm>

using namespace logid::features;
using namespace logid::backend;

static const uint8_t kMaxEffect = 14;

static uint8_t clampStrength(int strength) {
    return std::max(1, std::min(strength, 100));
}

class HapticFeedback::IPC : public ipcgull::interface {
public:
    explicit IPC(HapticFeedback* parent) : ipcgull::interface(
            SERVICE_ROOT_NAME ".HapticFeedback", {
                {"GetEnabled",      {this, &IPC::getEnabled,      {"enabled"}}},
                {"SetEnabled",      {this, &IPC::setEnabled,      {"enabled"}}},
                {"GetStrength",     {this, &IPC::getStrength,     {"strength"}}},
                {"SetStrength",     {this, &IPC::setStrength,     {"strength"}}},
                {"GetBatterySaving",{this, &IPC::getBatterySaving,{"battery_saving"}}},
                {"SetBatterySaving",{this, &IPC::setBatterySaving,{"battery_saving"}}},
                {"PlayEffect",      {this, &IPC::playEffect,      {"effect"}}}
            }, {}, {}), _parent(*parent) {
    }

    bool getEnabled() const { return _parent._enabled.load(); }

    void setEnabled(bool enabled) {
        _parent._enabled = enabled;
        _parent.configure();
    }

    uint8_t getStrength() const { return _parent._strength.load(); }

    void setStrength(uint8_t strength) {
        _parent._strength = clampStrength(strength);
        _parent.configure();
    }

    bool getBatterySaving() const { return _parent._battery_saving.load(); }

    void setBatterySaving(bool battery_saving) {
        _parent._battery_saving = battery_saving;
        _parent.configure();
    }

    void playEffect(uint8_t effect) { _parent.playEffect(effect); }

private:
    HapticFeedback& _parent;
};

HapticFeedback::HapticFeedback(Device* device) : DeviceFeature(device) {
    try {
        _haptic_feedback = std::make_shared<hidpp20::HapticFeedback>(&device->hidpp20());
    } catch (hidpp20::UnsupportedFeature& e) {
        throw UnsupportedFeature();
    }

    _ipc_interface = _device->ipcNode()->make_interface<IPC>(this);
}

void HapticFeedback::configure() {
    setStrength(_strength.load(), _enabled.load(), _battery_saving.load());
}

void HapticFeedback::listen() {
}

void HapticFeedback::setProfile(config::Profile&) {
    // No config file integration — settings managed via D-Bus only
}

void HapticFeedback::setStrength(uint8_t strength, bool enabled, bool battery_saving) {
    _haptic_feedback->setStrength(clampStrength(strength), enabled, battery_saving);
}

void HapticFeedback::playEffect(uint8_t effect) {
    if (_enabled.load() && effect <= kMaxEffect) {
        _haptic_feedback->playEffect(effect);
    }
}
