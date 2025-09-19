var types_8h =
[
    [ "system_status_t", "d7/dc4/structsystem__status__t.html", "d7/dc4/structsystem__status__t" ],
    [ "modbus_frame_t", "db/dc0/structmodbus__frame__t.html", "db/dc0/structmodbus__frame__t" ],
    [ "safety_event_log_t", "dc/d5b/structsafety__event__log__t.html", "dc/d5b/structsafety__event__log__t" ],
    [ "sonicator_hardware_config_t", "d2/d25/structsonicator__hardware__config__t.html", "d2/d25/structsonicator__hardware__config__t" ],
    [ "diagnostics_metrics_t", "d3/dd5/structdiagnostics__metrics__t.html", "d3/dd5/structdiagnostics__metrics__t" ],
    [ "MAX_ACCEPTABLE_LOOP_TIME_US", "d9/d49/types_8h.html#a59ae6fbc737635c08aee64e3de257d36", null ],
    [ "MAX_ERROR_DESCRIPTION_LENGTH", "d9/d49/types_8h.html#ac1eac485f524973d8f37410bbbba29fc", null ],
    [ "MAX_MODBUS_FRAME_SIZE", "d9/d49/types_8h.html#ae13fb6cbf3f5eb5a55230133ed2a2c92", null ],
    [ "MAX_MODBUS_RESPONSE_TIME_US", "d9/d49/types_8h.html#aac24cc0d01cf1d82a0896b9ba26dc5d3", null ],
    [ "MAX_MODULE_NAME_LENGTH", "d9/d49/types_8h.html#a16b0016b468df936da40743a62b4a2d0", null ],
    [ "MAX_SAFETY_EVENT_LOG_ENTRIES", "d9/d49/types_8h.html#a1d9bd7e7dc0195463abf28b5271a9c5b", null ],
    [ "MAX_VALID_AMPLITUDE_PERCENT", "d9/d49/types_8h.html#a5075f8b1e5ab01e5c1b6da79df5c4e1a", null ],
    [ "MAX_VALID_FREQUENCY_HZ", "d9/d49/types_8h.html#a0a53f1a64d59fec2e98829145301a317", null ],
    [ "MAX_VALID_POWER_WATTS", "d9/d49/types_8h.html#aa1a402f7e264b32ea6504618594d269d", null ],
    [ "MIN_MODBUS_RESPONSE_TIME_US", "d9/d49/types_8h.html#a65d5ca1d014a3b9a604c2aca65711df3", null ],
    [ "MIN_VALID_AMPLITUDE_PERCENT", "d9/d49/types_8h.html#a810d719081a1e85f300a4109054a0343", null ],
    [ "MIN_VALID_FREQUENCY_HZ", "d9/d49/types_8h.html#a3139e8684f9794262c232d838b002e1a", null ],
    [ "MIN_VALID_POWER_WATTS", "d9/d49/types_8h.html#a4447fc1370b6e0464d1077b2d48a9b91", null ],
    [ "error_notification_callback_t", "d9/d49/types_8h.html#a3aecb52560f25e335b14a24291bfeaaa", null ],
    [ "modbus_register_callback_t", "d9/d49/types_8h.html#a5d685fcbe0b5481ba6f61a0669210084", null ],
    [ "safety_event_callback_t", "d9/d49/types_8h.html#a1ccd0b4ac559edcdc9963b4d2708f375", null ],
    [ "modbus_function_t", "d9/d49/types_8h.html#a015c3588744a0406fafc81fecb3eddfd", [
      [ "MODBUS_FUNC_READ_HOLDING_REGISTERS", "d9/d49/types_8h.html#a015c3588744a0406fafc81fecb3eddfda45071a1751fc71d424b7b639b03a4377", null ],
      [ "MODBUS_FUNC_WRITE_SINGLE_REGISTER", "d9/d49/types_8h.html#a015c3588744a0406fafc81fecb3eddfda9fafa7e8ac52121d0f458f635e9c21cc", null ],
      [ "MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS", "d9/d49/types_8h.html#a015c3588744a0406fafc81fecb3eddfda894ba7dc5dc8061c14150b4cfaddb3cb", null ]
    ] ],
    [ "safety_event_t", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17", [
      [ "SAFETY_EVENT_NONE", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17aef847846a908427acf9943865820c8af", null ],
      [ "SAFETY_EVENT_EMERGENCY_STOP", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17aa916098bab572ce0e56dd72098970f14", null ],
      [ "SAFETY_EVENT_OVERLOAD_DETECTED", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17ad9a3e628a6414e395bac93f4db580f02", null ],
      [ "SAFETY_EVENT_FREQUENCY_LOCK_LOST", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17a69e682d24fee34f6913505d236440f12", null ],
      [ "SAFETY_EVENT_POWER_FAULT", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17a12c38dd39f00d9aee894ab7c6cfe6923", null ],
      [ "SAFETY_EVENT_COMMUNICATION_TIMEOUT", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17a805859073dcd71ab349ad315a62db61d", null ],
      [ "SAFETY_EVENT_WATCHDOG_TIMEOUT", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17af3d3df530924f0f1c85a7b876913e18a", null ],
      [ "SAFETY_EVENT_SYSTEM_FAULT", "d9/d49/types_8h.html#a90bdcf2de6b5ad6ab40abac0b872ba17ac18fd24a02f4c590141013434299376e", null ]
    ] ],
    [ "system_mode_t", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627d", [
      [ "SYSTEM_MODE_INIT", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627da6283084993ae74b2bafd95e0122af9db", null ],
      [ "SYSTEM_MODE_NORMAL", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627da3a1be6cdf32c139caca02cab153f375e", null ],
      [ "SYSTEM_MODE_SAFETY", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627da76b5230a9956b2906e1984afa6726d19", null ],
      [ "SYSTEM_MODE_MAINTENANCE", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627daa20d7f19f485c84a41680d91b139c1b8", null ],
      [ "SYSTEM_MODE_ERROR", "d9/d49/types_8h.html#a6e4cea4b6246f491c526211f5405627da8635d2327dabfaa62a927aa2445f39aa", null ]
    ] ]
];