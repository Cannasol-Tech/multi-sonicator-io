"""
pytest unit tests for Multi-Sonicator I/O Controller business logic
Organizational standard compliant - uses pytest for all unit testing
"""

import pytest
from cffi import FFI
from pathlib import Path

# Build CFFI interface for C business logic
ffi = FFI()

# Define C interface for business logic functions
ffi.cdef("""
    // Multi-unit controller state
    typedef struct {
        uint8_t amplitude[4];
        uint16_t frequency[4];
        uint8_t active_mask;
        uint8_t active_count;
        bool overload_detected[4];
        bool emergency_stop;
    } sonicator_controller_t;
    
    // MODBUS protocol functions
    bool validate_modbus_register_address(uint16_t address);
    uint16_t calculate_modbus_crc16(uint8_t* data, uint8_t length);
    bool parse_modbus_frame(uint8_t* frame, uint8_t length, uint16_t* address, uint16_t* value);
    
    // Multi-unit orchestration functions  
    uint8_t calculate_active_count(uint8_t mask);
    bool update_active_mask(sonicator_controller_t* controller, uint8_t unit, bool active);
    
    // Safety and validation functions
    uint8_t clamp_amplitude(uint8_t input);
    bool validate_amplitude_range(uint8_t amplitude);
    bool is_overload_condition(uint16_t power_reading);
    bool should_emergency_shutdown(sonicator_controller_t* controller);
    
    // Configuration and initialization
    void initialize_controller(sonicator_controller_t* controller);
    bool validate_system_configuration(void);
""")

# Global library reference (will be set by conftest.py)
lib = None

def get_lib():
    """Get the compiled C library reference."""
    global lib
    if lib is None:
        # Try to load pre-compiled library
        try:
            lib = ffi.dlopen("./build/libsonicator_logic.so")
        except OSError:
            pytest.skip("C library not compiled. Run: gcc -shared -fPIC -DUNIT_TEST src/*.c -o build/libsonicator_logic.so")
    return lib


class TestMODBUSProtocol:
    """Test MODBUS RTU protocol business logic."""
    
    def test_register_address_validation(self):
        """Test MODBUS register address validation (FR8)."""
        lib = get_lib()
        
        # Valid amplitude registers (40001-40004)
        assert lib.validate_modbus_register_address(40001) == True
        assert lib.validate_modbus_register_address(40002) == True  
        assert lib.validate_modbus_register_address(40003) == True
        assert lib.validate_modbus_register_address(40004) == True
        
        # Valid start/stop registers (40005-40008)
        assert lib.validate_modbus_register_address(40005) == True
        assert lib.validate_modbus_register_address(40008) == True
        
        # Valid status registers (40021-40024)
        assert lib.validate_modbus_register_address(40021) == True
        assert lib.validate_modbus_register_address(40024) == True
        
        # Invalid addresses
        assert lib.validate_modbus_register_address(30000) == False
        assert lib.validate_modbus_register_address(50000) == False
        assert lib.validate_modbus_register_address(40000) == False  # Below range
        assert lib.validate_modbus_register_address(40025) == False  # Above range
    
    def test_crc16_calculation(self):
        """Test MODBUS CRC-16 calculation algorithm."""
        lib = get_lib()
        
        # Known test vectors for MODBUS CRC-16
        test_cases = [
            (bytes([0x01, 0x03, 0x00, 0x01, 0x00, 0x01]), 0xD5CA),
            (bytes([0x02, 0x03, 0x9C, 0x41, 0x00, 0x01]), 0x8B8E),
            (bytes([0x01, 0x06, 0x9C, 0x41, 0x00, 0x4B]), 0x9C4E),
        ]
        
        for frame_data, expected_crc in test_cases:
            calculated_crc = lib.calculate_modbus_crc16(frame_data, len(frame_data))
            assert calculated_crc == expected_crc, f"CRC mismatch for {frame_data.hex()}"
    
    def test_frame_parsing(self):
        """Test MODBUS frame parsing logic."""
        lib = get_lib()
        
        # Test valid read holding register frame
        frame = bytes([0x02, 0x03, 0x9C, 0x41, 0x00, 0x01, 0x8B, 0x8E])
        address = ffi.new("uint16_t*")
        value = ffi.new("uint16_t*")
        
        result = lib.parse_modbus_frame(frame, len(frame), address, value)
        assert result == True
        assert address[0] == 40001  # 0x9C41 = 40001
        assert value[0] == 1        # Read count
        
        # Test invalid frame (wrong CRC)
        bad_frame = bytes([0x02, 0x03, 0x9C, 0x41, 0x00, 0x01, 0x00, 0x00])
        result = lib.parse_modbus_frame(bad_frame, len(bad_frame), address, value)
        assert result == False


class TestMultiUnitOrchestration:
    """Test multi-unit sonicator orchestration logic."""
    
    def test_active_count_calculation(self):
        """Test active sonicator count calculation (FR9)."""
        lib = get_lib()
        
        # Test all possible combinations
        assert lib.calculate_active_count(0b0000) == 0  # No units active
        assert lib.calculate_active_count(0b0001) == 1  # Unit 1 only
        assert lib.calculate_active_count(0b0010) == 1  # Unit 2 only  
        assert lib.calculate_active_count(0b0100) == 1  # Unit 3 only
        assert lib.calculate_active_count(0b1000) == 1  # Unit 4 only
        assert lib.calculate_active_count(0b0011) == 2  # Units 1,2
        assert lib.calculate_active_count(0b1010) == 2  # Units 2,4
        assert lib.calculate_active_count(0b0111) == 3  # Units 1,2,3
        assert lib.calculate_active_count(0b1111) == 4  # All units
    
    def test_active_mask_update(self):
        """Test active mask update logic."""
        lib = get_lib()
        
        controller = ffi.new("sonicator_controller_t*")
        lib.initialize_controller(controller)
        
        # Start units individually
        assert lib.update_active_mask(controller, 1, True) == True
        assert controller.active_mask == 0b0001
        assert controller.active_count == 1
        
        assert lib.update_active_mask(controller, 3, True) == True  
        assert controller.active_mask == 0b0101  # Units 1,3
        assert controller.active_count == 2
        
        # Stop unit 1
        assert lib.update_active_mask(controller, 1, False) == True
        assert controller.active_mask == 0b0100  # Unit 3 only
        assert controller.active_count == 1
        
        # Test invalid unit (should fail)
        assert lib.update_active_mask(controller, 5, True) == False
        assert lib.update_active_mask(controller, 0, True) == False


class TestAmplitudeValidation:
    """Test amplitude validation and clamping logic."""
    
    def test_amplitude_clamping(self):
        """Test amplitude clamping to 20-100% range (FR3)."""
        lib = get_lib()
        
        # Test clamping to minimum (20%)
        assert lib.clamp_amplitude(0) == 20
        assert lib.clamp_amplitude(10) == 20
        assert lib.clamp_amplitude(19) == 20
        
        # Test valid range (no clamping)
        assert lib.clamp_amplitude(20) == 20
        assert lib.clamp_amplitude(50) == 50
        assert lib.clamp_amplitude(75) == 75
        assert lib.clamp_amplitude(100) == 100
        
        # Test clamping to maximum (100%)
        assert lib.clamp_amplitude(101) == 100
        assert lib.clamp_amplitude(150) == 100
        assert lib.clamp_amplitude(255) == 100
    
    def test_amplitude_range_validation(self):
        """Test amplitude range validation logic."""
        lib = get_lib()
        
        # Valid range (20-100%)
        assert lib.validate_amplitude_range(20) == True
        assert lib.validate_amplitude_range(50) == True
        assert lib.validate_amplitude_range(100) == True
        
        # Invalid range
        assert lib.validate_amplitude_range(19) == False
        assert lib.validate_amplitude_range(101) == False
        assert lib.validate_amplitude_range(0) == False
        assert lib.validate_amplitude_range(255) == False


class TestSafetyLogic:
    """Test safety and monitoring algorithms."""
    
    def test_overload_detection(self):
        """Test overload detection algorithm."""
        lib = get_lib()
        
        # Normal power readings (not overload)
        assert lib.is_overload_condition(0) == False     # No power
        assert lib.is_overload_condition(500) == False   # Low power
        assert lib.is_overload_condition(1500) == False  # Normal power
        assert lib.is_overload_condition(2000) == False  # High normal
        
        # Overload conditions (assuming threshold around 2200W)
        assert lib.is_overload_condition(2300) == True   # Overload
        assert lib.is_overload_condition(3000) == True   # High overload
        assert lib.is_overload_condition(4095) == True   # Max ADC reading
    
    def test_emergency_shutdown_logic(self):
        """Test emergency shutdown decision logic."""
        lib = get_lib()
        
        controller = ffi.new("sonicator_controller_t*")
        lib.initialize_controller(controller)
        
        # Normal operation - no shutdown
        controller.overload_detected[0] = False
        controller.overload_detected[1] = False
        controller.emergency_stop = False
        assert lib.should_emergency_shutdown(controller) == False
        
        # Single unit overload - should trigger shutdown
        controller.overload_detected[0] = True
        assert lib.should_emergency_shutdown(controller) == True
        
        # Emergency stop activated - should trigger shutdown
        controller.overload_detected[0] = False
        controller.emergency_stop = True
        assert lib.should_emergency_shutdown(controller) == True


class TestSystemConfiguration:
    """Test system configuration and initialization."""
    
    def test_controller_initialization(self):
        """Test controller initialization to safe defaults (FR10)."""
        lib = get_lib()
        
        controller = ffi.new("sonicator_controller_t*")
        lib.initialize_controller(controller)
        
        # All units should start inactive
        assert controller.active_mask == 0
        assert controller.active_count == 0
        
        # All amplitudes should be at minimum safe value
        for i in range(4):
            assert controller.amplitude[i] == 20  # Minimum amplitude
            assert controller.overload_detected[i] == False
        
        # Emergency stop should be inactive
        assert controller.emergency_stop == False
    
    def test_system_configuration_validation(self):
        """Test system configuration validation."""
        lib = get_lib()
        
        # Should validate that system is properly configured
        assert lib.validate_system_configuration() == True


# Parametrized tests for comprehensive edge case coverage
@pytest.mark.parametrize("register,expected", [
    (40001, True),   # Unit 1 amplitude
    (40002, True),   # Unit 2 amplitude  
    (40003, True),   # Unit 3 amplitude
    (40004, True),   # Unit 4 amplitude
    (40005, True),   # Unit 1 start/stop
    (40006, True),   # Unit 2 start/stop
    (40007, True),   # Unit 3 start/stop
    (40008, True),   # Unit 4 start/stop
    (40021, True),   # Unit 1 status
    (40022, True),   # Unit 2 status
    (40023, True),   # Unit 3 status
    (40024, True),   # Unit 4 status
    (30000, False),  # Invalid - below range
    (50000, False),  # Invalid - above range
    (40000, False),  # Invalid - boundary
    (40025, False),  # Invalid - boundary
])
def test_modbus_register_validation_comprehensive(register, expected):
    """Comprehensive MODBUS register validation test."""
    lib = get_lib()
    assert lib.validate_modbus_register_address(register) == expected


@pytest.mark.parametrize("mask,expected_count", [
    (0b0000, 0), (0b0001, 1), (0b0010, 1), (0b0011, 2),
    (0b0100, 1), (0b0101, 2), (0b0110, 2), (0b0111, 3),
    (0b1000, 1), (0b1001, 2), (0b1010, 2), (0b1011, 3),
    (0b1100, 2), (0b1101, 3), (0b1110, 3), (0b1111, 4),
])
def test_active_count_comprehensive(mask, expected_count):
    """Comprehensive active count calculation test."""
    lib = get_lib()
    assert lib.calculate_active_count(mask) == expected_count


@pytest.mark.parametrize("amplitude,expected", [
    (0, 20), (10, 20), (19, 20),     # Below minimum → clamp to 20
    (20, 20), (50, 50), (100, 100),  # Valid range → unchanged
    (101, 100), (150, 100), (255, 100),  # Above maximum → clamp to 100
])
def test_amplitude_clamping_comprehensive(amplitude, expected):
    """Comprehensive amplitude clamping test."""
    lib = get_lib()
    assert lib.clamp_amplitude(amplitude) == expected
