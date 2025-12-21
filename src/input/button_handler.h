#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "../config.h"
#include <ESP32ButtonHandler.h>

// Forward declaration of callback interface
class ButtonHandlerCallbacks {
public:
    virtual void onButton1Click() = 0;
    virtual void onButton2Click() = 0;
    virtual void onButton1LongPress() = 0;
    virtual void onButton2LongPress() = 0;
};

class ButtonHandler {
public:
    static ButtonHandler& getInstance();

    // Initialize button handling
    bool begin(ButtonHandlerCallbacks* callbacks);

private:
    ButtonHandler();
    ~ButtonHandler();

    // Prevent copying
    ButtonHandler(const ButtonHandler&) = delete;
    ButtonHandler& operator=(const ButtonHandler&) = delete;

    ESP32ButtonHandler* button1;
    ESP32ButtonHandler* button2;
    ButtonHandlerCallbacks* appCallbacks;
    bool initialized;
};

#endif // BUTTON_HANDLER_H
