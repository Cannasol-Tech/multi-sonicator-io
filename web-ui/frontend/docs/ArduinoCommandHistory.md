# Arduino Command History Feature

## Overview

The Arduino Command History feature provides real-time tracking and display of Arduino commands sent and responses received in the Multi-Sonicator IO web interface. This feature enhances debugging capabilities and provides visibility into the communication flow between the web interface and the Arduino hardware.

## Features

- **Real-time Command Tracking**: Displays the last 5 command/response pairs in real-time
- **Connection Status Indicator**: Shows live connection status with visual indicators
- **Response Time Monitoring**: Color-coded response time indicators for performance monitoring
- **Error Handling**: Clear display of command errors and timeout situations
- **Interactive Elements**: Optional click handlers for command entries
- **Responsive Design**: Adapts to different screen sizes and integrates seamlessly with existing UI

## Architecture

### Components

#### ArduinoCommandHistory
**Location**: `src/components/ArduinoCommandHistory.tsx`

The main component that renders the command history interface.

**Props**:
```typescript
interface ArduinoCommandHistoryProps {
  commandPairs: CommandPair[]     // Array of command/response pairs
  connected: boolean              // Connection status
  maxEntries?: number            // Maximum entries to display (default: 5)
  onCommandClick?: (command: ArduinoCommand) => void  // Optional click handler
}
```

#### useArduinoCommandLog Hook
**Location**: `src/hooks/useArduinoCommandLog.ts`

Custom hook for managing command history state and operations.

**Interface**:
```typescript
interface UseArduinoCommandLogReturn {
  addCommandPair: (command: string, response?: string, responseTime?: number, error?: string) => void
  getRecentPairs: (limit?: number) => CommandPair[]
  clearHistory: () => void
}
```

### Data Structures

#### ArduinoCommand
```typescript
interface ArduinoCommand {
  id: string                    // Unique identifier
  direction: 'sent' | 'received' // Command direction
  data: string                  // Command/response text
  timestamp: number             // Unix timestamp
  type: 'command' | 'response'  // Command type
  status?: 'success' | 'error' | 'pending' // Status indicator
  responseTime?: number         // Response time in milliseconds
  error?: string               // Error message if applicable
}
```

#### CommandPair
```typescript
interface CommandPair {
  sent: ArduinoCommand         // Sent command
  received?: ArduinoCommand    // Received response (optional)
}
```

## Integration Points

### HardwareDiagram Component
The ArduinoCommandHistory component is integrated into the HardwareDiagram component, positioned below the Pin Connections display.

**Integration Code**:
```typescript
// Arduino command logging
const { getRecentPairs } = useArduinoCommandLog({ maxEntries: 50 })

// Component rendering
<ArduinoCommandHistory 
  commandPairs={getRecentPairs(5)}
  connected={hardwareState.connection.connected}
  maxEntries={5}
/>
```

### App Component Integration
The main App component handles command logging through WebSocket integration and user interactions.

**Key Integration Points**:
1. **Pin Control Commands**: Logged when users interact with pin controls
2. **Ping Commands**: Logged when hardware ping is triggered
3. **WebSocket Responses**: Real-time response handling via WebSocket messages

## Styling

### CSS Classes
The component uses a comprehensive set of CSS classes for styling:

- `.arduino-command-history` - Main container
- `.command-history-header` - Header section with title and connection status
- `.command-history-content` - Scrollable content area
- `.command-pairs-list` - List of command pairs
- `.command-entry` - Individual command/response entries
- `.connection-indicator` - Connection status indicator
- `.command-history-footer` - Statistics footer

### Visual Indicators

#### Connection Status
- **Live**: Green dot with "Live" text for active connections
- **Offline**: Red dot with "Offline" text for disconnected state

#### Response Time Colors
- **🟢 Green**: < 20ms (Excellent)
- **🟡 Yellow**: 20-100ms (Good)
- **🟠 Orange**: 100-500ms (Slow)
- **🔴 Red**: > 500ms (Very Slow)

#### Status Icons
- **✅**: Successful command/response
- **❌**: Error or failed command
- **⏳**: Pending response

## Usage Examples

### Basic Usage
```typescript
import ArduinoCommandHistory from './components/ArduinoCommandHistory'
import { useArduinoCommandLog } from './hooks/useArduinoCommandLog'

function MyComponent() {
  const { getRecentPairs } = useArduinoCommandLog({ maxEntries: 10 })
  
  return (
    <ArduinoCommandHistory
      commandPairs={getRecentPairs(5)}
      connected={true}
      maxEntries={5}
    />
  )
}
```

### With Click Handler
```typescript
const handleCommandClick = (command: ArduinoCommand) => {
  console.log('Command clicked:', command)
  // Handle command click (e.g., show details, retry command)
}

<ArduinoCommandHistory
  commandPairs={commandPairs}
  connected={connected}
  maxEntries={5}
  onCommandClick={handleCommandClick}
/>
```

### Adding Commands Programmatically
```typescript
const { addCommandPair } = useArduinoCommandLog({ maxEntries: 50 })

// Add successful command
addCommandPair('SET_PIN_HIGH D2', 'OK - Pin D2 set to HIGH', 25)

// Add failed command
addCommandPair('INVALID_COMMAND', 'ERROR - Unknown command', 100, 'Command not recognized')
```

## Testing

### Unit Tests
**Location**: `src/components/__tests__/ArduinoCommandHistory.test.tsx`

Comprehensive unit tests covering:
- Component rendering with different props
- Command display and formatting
- Connection status indicators
- Response time color coding
- Error handling
- User interactions
- Accessibility features

**Coverage**: 100% statement coverage, 89.18% branch coverage

### Integration Tests
**Location**: `src/components/__tests__/ArduinoCommandHistory.integration.test.tsx`

Integration tests covering:
- Component integration with command pairs
- Real-time updates
- Connection status changes
- maxEntries limit handling
- Click event handling

### Running Tests
```bash
# Run unit tests
npm test -- ArduinoCommandHistory.test.tsx

# Run integration tests
npm test -- ArduinoCommandHistory.integration.test.tsx

# Run with coverage
npm test -- ArduinoCommandHistory.test.tsx --coverage
```

## Performance Considerations

### Memory Management
- Command history is limited by `maxEntries` parameter to prevent memory leaks
- Old commands are automatically removed when limit is exceeded
- Efficient array slicing for display purposes

### Rendering Optimization
- Uses React's built-in optimization for list rendering
- Minimal re-renders through proper prop handling
- Efficient timestamp formatting

### WebSocket Integration
- Non-blocking command logging
- Efficient message handling
- Graceful error handling for connection issues

## Accessibility

### ARIA Support
- Proper semantic HTML structure
- Heading hierarchy for screen readers
- Code elements for command text

### Keyboard Navigation
- Clickable elements are keyboard accessible
- Proper focus management
- Screen reader friendly content

## Browser Compatibility

- Modern browsers supporting ES2018+
- CSS Grid and Flexbox support required
- WebSocket support required for real-time features

## Future Enhancements

### Planned Features
1. **Command Filtering**: Filter commands by type or status
2. **Export Functionality**: Export command history to file
3. **Search Capability**: Search through command history
4. **Command Replay**: Replay previous commands
5. **Performance Metrics**: Detailed performance analytics

### Configuration Options
1. **Customizable Colors**: User-defined response time thresholds
2. **Display Options**: Configurable information display
3. **Persistence**: Optional command history persistence

## Troubleshooting

### Common Issues

#### Commands Not Appearing
- Verify WebSocket connection is active
- Check that `addCommandPair` is being called correctly
- Ensure component is receiving updated `commandPairs` prop

#### Styling Issues
- Verify CSS classes are properly imported
- Check for CSS conflicts with existing styles
- Ensure CSS variables are defined

#### Performance Issues
- Reduce `maxEntries` if experiencing slow rendering
- Check for memory leaks in command logging
- Verify efficient prop passing

### Debug Mode
Enable debug logging by setting:
```typescript
const { addCommandPair } = useArduinoCommandLog({ 
  maxEntries: 50, 
  debug: true 
})
```

## Contributing

### Code Standards
- Follow existing React component patterns
- Maintain TypeScript strict mode compliance
- Include comprehensive tests for new features
- Update documentation for any changes

### Pull Request Requirements
- All tests must pass
- Code coverage must remain above 85%
- Documentation must be updated
- Integration tests must be included for new features

## License

This feature is part of the Multi-Sonicator IO project and follows the same licensing terms.
