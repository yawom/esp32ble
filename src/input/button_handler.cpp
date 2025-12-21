#include "button_handler.h"

ButtonHandler& ButtonHandler::getInstance() {
    static ButtonHandler instance;
    return instance;
}

ButtonHandler::ButtonHandler()
    : button1(nullptr)
    , button2(nullptr)
    , appCallbacks(nullptr)
    , initialized(false) {
}

ButtonHandler::~ButtonHandler() {
    if (button1) {
        delete button1;
    }
    if (button2) {
        delete button2;
    }
}

bool ButtonHandler::begin(ButtonHandlerCallbacks* callbacks) {
    if (initialized) {
        return true;
    }

    appCallbacks = callbacks;

    DEBUG_PRINTLN("Initializing button handler...");

    // Initialize Button 1 (Left button)
    // ESP32ButtonHandler(pin, activeLow, pullUp, holdThreshold, multiClickThreshold, debounceDelay)
    button1 = new ESP32ButtonHandler(
        BUTTON_1_PIN,
        true,                          // activeLow
        true,                          // pullUp
        LONG_PRESS_DURATION_MS,        // holdThreshold (5000ms for long press)
        250,                           // multiClickThreshold
        BUTTON_DEBOUNCE_MS             // debounceDelay
    );

    if (!button1 || !button1->isInitialized()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize Button 1");
        if (button1) {
            delete button1;
            button1 = nullptr;
        }
        return false;
    }

    // Initialize Button 2 (Right button)
    button2 = new ESP32ButtonHandler(
        BUTTON_2_PIN,
        true,                          // activeLow
        true,                          // pullUp
        LONG_PRESS_DURATION_MS,        // holdThreshold (5000ms for long press)
        250,                           // multiClickThreshold
        BUTTON_DEBOUNCE_MS             // debounceDelay
    );

    if (!button2 || !button2->isInitialized()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize Button 2");
        if (button2) {
            delete button2;
            button2 = nullptr;
        }
        delete button1;
        button1 = nullptr;
        return false;
    }

    // Set up callbacks using lambdas
    button1->setOnClickCallback([this](ESP32ButtonHandler* handler, int clickCount) {
        DEBUG_PRINTF("Button 1 clicked (count: %d)\n", clickCount);
        if (appCallbacks) {
            appCallbacks->onButton1Click();
        }
    });

    button1->setOnLongPressStartCallback([this](ESP32ButtonHandler* handler) {
        DEBUG_PRINTLN("Button 1 long press");
        if (appCallbacks) {
            appCallbacks->onButton1LongPress();
        }
    });

    button2->setOnClickCallback([this](ESP32ButtonHandler* handler, int clickCount) {
        DEBUG_PRINTF("Button 2 clicked (count: %d)\n", clickCount);
        if (appCallbacks) {
            appCallbacks->onButton2Click();
        }
    });

    button2->setOnLongPressStartCallback([this](ESP32ButtonHandler* handler) {
        DEBUG_PRINTLN("Button 2 long press");
        if (appCallbacks) {
            appCallbacks->onButton2LongPress();
        }
    });

    initialized = true;
    DEBUG_PRINTLN("Button handler initialized successfully");

    return true;
}
