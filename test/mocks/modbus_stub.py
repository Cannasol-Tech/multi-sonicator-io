"""
MODBUS Client Stub for Integration and HIL Testing

Purpose:
- Simulates a MODBUS client for testing without real hardware.
- Allows setting and getting register values.
- Can be extended to simulate errors and other scenarios.
"""

class ModbusClientStub:
    def __init__(self):
        self.registers = {}

    def write_register(self, address, value):
        """Simulates writing a value to a single register."""
        self.registers[address] = value
        print(f"STUB: Wrote {value} to register {address}")
        return True

    def read_register(self, address):
        """Simulates reading a value from a single register."""
        value = self.registers.get(address, 0)
        print(f"STUB: Read {value} from register {address}")
        return value

    def write_registers(self, address, values):
        """Simulates writing values to multiple registers."""
        for i, value in enumerate(values):
            self.registers[address + i] = value
        print(f"STUB: Wrote {len(values)} values starting at register {address}")
        return True

    def read_registers(self, address, count):
        """Simulates reading values from multiple registers."""
        values = [self.registers.get(address + i, 0) for i in range(count)]
        print(f"STUB: Read {count} values starting at register {address}")
        return values
