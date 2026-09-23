# Melatonin Inspector MCP Bridge

This directory contains the Model Context Protocol (MCP) Bridge for `melatonin_inspector`.

By running this bridge alongside your JUCE application, you can expose the internal UI state of your application to AI coding assistants (like Claude Desktop or custom agents), allowing them to dynamically inspect, interact with, and manipulate your application's graphical user interface.

## Architecture

The system is composed of two main parts:

1. **JUCE IPC Server (C++)**: An embedded TCP server running inside `melatonin_inspector` on `127.0.0.1` port `8484`. It listens for incoming JSON-RPC style messages over JUCE's `InterprocessConnection` protocol and executes them on the JUCE Message Thread.
2. **MCP Bridge (TypeScript)**: A standalone Node.js process that communicates with the C++ IPC Server via TCP. It acts as an MCP Server, translating standard MCP Tool definitions into the raw IPC commands understood by the JUCE application.

## 1. Setup Your JUCE Application

To use the MCP Bridge, your JUCE application must link the `melatonin_inspector` module and initialize the inspector.

1. Add `melatonin_inspector` to your JUCE project (via CMake or Projucer).
2. Instantiate the `melatonin::Inspector` in your application (usually inside your `MainComponent` or `MainWindow`).

```cpp
#include <melatonin_inspector/melatonin_inspector.h>

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow (juce::String name) : DocumentWindow (name, juce::Colours::lightgrey, DocumentWindow::allButtons)
    {
        // ... configure your window ...

        // Constructing the inspector starts the IPC Server on port 8484.
        // The second parameter `true` also opens the inspector at startup.
        inspector.setVisible(true);
    }

private:
    // ...
    melatonin::Inspector inspector { *this, true };
};
```

As soon as the `melatonin::Inspector` is constructed it listens for connections from this machine on port `8484`, whether or not the inspector window is open.

## 2. Setup the MCP Bridge

Ensure you have Node.js (v18+) installed.

1. Navigate to this directory:
   ```bash
   cd mcp_bridge
   ```
2. Install the dependencies:
   ```bash
   npm install
   ```
3. Build the TypeScript code:
   ```bash
   npm run build
   ```

## 3. Running the MCP Bridge

You can run the MCP bridge using the standard `stdio` transport. It is designed to be invoked by an MCP Host (like the official MCP Inspector or Claude Desktop).

### Testing Interactively

To test the tools interactively in your browser using the official MCP inspector:

```bash
npx @modelcontextprotocol/inspector node build/index.js
```

### Using with Claude Desktop

To expose your JUCE application's UI to Claude, add this server to your `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "melatonin_inspector": {
      "command": "node",
      "args": [
        "/absolute/path/to/melatonin_inspector/mcp_bridge/build/index.js"
      ]
    }
  }
}
```

### Using with Antigravity

If you are using **Antigravity** (the agentic AI coding assistant), you can add the server by editing your MCP configuration file (typically located at `~/.gemini/antigravity/mcp.json` or `~/.gemini/antigravity/mcp_servers.json`).

Add the following entry to the configuration:

```json
{
  "mcpServers": {
    "melatonin_inspector": {
      "command": "node",
      "args": [
        "/absolute/path/to/melatonin_inspector/mcp_bridge/build/index.js"
      ]
    }
  }
}
```

Restart Antigravity, and it will immediately gain the ability to inspect and edit your JUCE GUIs!

## Available MCP Tools

Once connected, the bridge exposes the following tools to the AI agent:

- **`juce_get_ui_tree`**: Returns the complete hierarchical tree of the application's components, including their IDs, types, visibility, bounds, and names.
- **`juce_get_component_details`**: Takes a specific component `id` and returns detailed properties (e.g., exact geometry, colors, LookAndFeel properties, text values).
- **`juce_highlight_component`**: Takes a component `id` and visually flashes it on the screen to help identify its location.
- **`juce_set_component_bounds`**: Modifies the bounds (X, Y, Width, Height) of a specific component `id` in real-time.
- **`juce_click_component`**: Simulates a mouse click on the center of a specified component `id` (useful for clicking buttons or toggles).
- **`juce_capture_screenshot`**: Takes a visual snapshot of a component `id` and returns it as a Base64-encoded PNG image natively through MCP's Image Content blocks.
- **`juce_quit_application`**: Gracefully requests the JUCE application to terminate.

## Troubleshooting

- **Connection Refused**: Ensure your JUCE application is running and has constructed a `melatonin::Inspector`. The bridge retries the connection every few seconds.
- **Port Conflicts**: The IPC Server binds to `127.0.0.1` port `8484`. If this port is in use, the C++ IPC server will fail to start.
