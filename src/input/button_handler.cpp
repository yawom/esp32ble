#include "button_handler.h"

// Static instance pointer for callbacks
static ButtonHandler* s_instance = nullptr;

ButtonHandler& ButtonHandler::getInstance() {
    static ButtonHandler instance;
    s_instance = &instance;
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
    button1 = new ESP32Button(BUTTON_1_PIN, BUTTON_DEBOUNCE_MS, true, true);  // Active LOW, internal pullup
    if (!button1) {
        DEBUG_PRINTLN("ERROR: Failed to create Button 1");
        return false;
    }

    // Initialize Button 2 (Right button)
    button2 = new ESP32Button(BUTTON_2_PIN, BUTTON_DEBOUNCE_MS, true, true);  // Active LOW, internal pullup
    if (!button2) {
        DEBUG_PRINTLN("ERROR: Failed to create Button 2");
        delete button1;
        button1 = nullptr;
        return false;
    }

    // Set up callbacks
    button1->setOnClickCallback(button1ClickCallback);
    button1->setOnLongPressCallback(button1LongPressCallback, LONG_PRESS_DURATION_MS);

    button2->setOnClickCallback(button2ClickCallback);
    button2->setOnLongPressCallback(button2LongPressCallback, LONG_PRESS_DURATION_MS);

    initialized = true;
    DEBUG_PRINTLN("Button handler initialized successfully");

    return true;
}

void ButtonHandler::update() {
    if (!initialized) {
        return;
    }

    // Update button states
    button1->update();
    button2->update();
}

// Static callback implementations
void ButtonHandler::button1ClickCallback() {
    DEBUG_PRINTLN("Button 1 clicked");
    if (s_instance && s_instance->appCallbacks) {
        s_instance->appCallbacks->onButton1Click();
    }
}

void ButtonHandler::button2ClickCallback() {
    DEBUG_PRINTLN("Button 2 clicked");
    if (s_instance && s_instance->appCallbacks) {
        s_instance->appCallbacks->onButton2Click();
    }
}

void ButtonHandler::button1LongPressCallback() {
    DEBUG_PRINTLN("Button 1 long press");
    if (s_instance && s_instance->appCallbacks) {
        s_instance->appCallbacks->onButton1LongPress();
    }
}

void ButtonHandler::button2LongPressCallback() {
    DEBUG_PRINTLN("Button 2 long press");
    if (s_instance && s_instance->appCallbacks) {
        s_instance->appCallbacks->onButton2LongPress();
    }
}
