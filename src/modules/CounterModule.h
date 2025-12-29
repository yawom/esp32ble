#pragma once

#include "boards/BoardProfiles.h"

#if HAS_DISPLAY

#include "modules/Module.h"
#include "../app/counter_app.h"
#include "../ble/ble_manager.h"

class CounterModule : public Module {
public:
    CounterModule(Logger* logger);
    ~CounterModule();

protected:
    void setup() override;
    void loop() override;
    void handleEvent(const ModuleEvent& event) override;

private:
    unsigned long lastUpdateTime;

    void draw();
};

#endif
