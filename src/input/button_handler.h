#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "../config.h"
#include <ESP32Button.h>

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

    // Update button states (call in loop)
    void update();

private:
    ButtonHandler();
    ~ButtonHandler();

    // Prevent copying
    ButtonHandler(const ButtonHandler&) = delete;
    ButtonHandler& operator=(const ButtonHandler&) = delete;

    ESP32Button* button1;
    ESP32Button* button2;
    ButtonHandlerCallbacks* appCallbacks;
    bool initialized;

    // Static callback functions (required by ESP32Button)
    static void button1ClickCallback();
    static void button2ClickCallback();
    static void button1LongPressCallback();
    static void button2LongPressCallback();
};

#endif // BUTTON_HANDLER_H
