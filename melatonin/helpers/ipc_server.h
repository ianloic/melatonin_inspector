#pragma once

#include "juce_core/juce_core.h"
#include "juce_data_structures/juce_data_structures.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "../component_model.h"
#include "component_helpers.h"

namespace melatonin
{
    class Inspector;

    class IpcConnection : public juce::InterprocessConnection
    {
    public:
        explicit IpcConnection (Inspector& inspectorRef)
            : juce::InterprocessConnection (true, 0x2172746a), inspector (inspectorRef) {}

        ~IpcConnection() override
        {
            // InterprocessConnection requires subclasses to disconnect before
            // they are destroyed, so no callback reaches a half-deleted object.
            disconnect();
        }

        void connectionMade() override {}
        void connectionLost() override {}

        void messageReceived (const juce::MemoryBlock& message) override;
        void handleMessage (const juce::var& json);
        void sendMessage (const juce::String& text);

    private:
        Inspector& inspector;

        juce::var serializeComponentTree (juce::Component* c);
        juce::Component* findComponentByPointerString (juce::Component* parent, const juce::String& ptrString);
        juce::DynamicObject::Ptr getDetails (juce::Component* c);
    };

    class IpcServer : public juce::InterprocessConnectionServer
    {
    public:
        explicit IpcServer (Inspector& inspectorRef) : inspector (inspectorRef) {}

        ~IpcServer() override
        {
            stop();
            activeConnections.clear();
        }

        juce::InterprocessConnection* createConnectionObject() override
        {
            auto* connection = new IpcConnection (inspector);
            activeConnections.add (connection);
            return connection;
        }

        // Listens on the loopback interface only: the protocol can click
        // buttons, resize components and quit the app, so it must not be
        // reachable from other machines.
        bool start (int port = 8484)
        {
            return beginWaitingForSocket (port, "127.0.0.1");
        }

    private:
        Inspector& inspector;
        juce::OwnedArray<IpcConnection> activeConnections;
    };
}
